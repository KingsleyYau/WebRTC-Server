/*
 * RtpSession.cpp
 *
 *  Created on: 2019/06/20
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include <common/LogManager.h>
#include <common/Math.h>
#include <common/CommonFunc.h>
#include <common/Arithmetic.h>

// libsrtp
#include <srtp.h>
#include <srtp_priv.h>
#include "RtpSession.h"

/*
 * SRTP key size
 */
#define MAX_KEY_LEN 96
static char kEmptyKey[MAX_KEY_LEN] = {0};

namespace mediaserver {

#define RTCP_HEADER_LENGTH 4
#define RTCP_COMMON_FEEDBACK_LENGTH 8

typedef struct {
    unsigned char cc : 4;      /* CSRC count             */
    unsigned char x : 1;       /* header extension flag  */
    unsigned char p : 1;       /* padding flag           */
    unsigned char version : 2; /* protocol version       */
    unsigned char pt : 7;      /* payload type           */
    unsigned char m : 1;       /* marker bit             */
    uint16_t seq;              /* sequence number        */
    uint32_t ts;               /* timestamp              */
    uint32_t ssrc;             /* synchronization source */
} RtpHeader; /* BIG END */

typedef struct {
	RtpHeader header;
    char body[RTP_MAX_PAYLOAD_LEN];
} RtpPacket;

// From RFC 3550, RTP: A Transport Protocol for Real-Time Applications.
//
// RTP header format.
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |V=2|P| RC/FMT  |      PT       |             length            |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
typedef struct {
    unsigned char rc : 5;      /* fmt 					 				 */
    unsigned char p : 1;       /* padding flag           				 */
    unsigned char version : 2; /* protocol version       				 */
    unsigned char pt : 8;      /* payload					 			 */
    uint16_t length;           /* count of media ssrc, each one is 32bit */
} RtcpHeader; /* BIG END */

typedef struct {
	RtcpHeader header;
	uint32_t ssrc;
} RtcpPacketCommon; /* BIG END */

// RFC 4585: Feedback format.
//
// Common packet format:
//
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |V=2|P|   FMT   |       PT      |          length               |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                  SSRC of packet sender                        |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                  SSRC of media source                         |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  :            Feedback Control Information (FCI)                 :
//  :                                                               :
// FMT Must be 1, PT must be PSFB
// RTPFB  |  205  | Transport layer FB message
// PSFB   |  206  | Payload-specific FB message
typedef struct {
	RtcpHeader header;
    uint32_t ssrc;
    uint32_t media_ssrc;
} RtcpPacketPLI; /* BIG END */

// RFC 4585: Feedback format.
// Common packet format:
//
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |V=2|P|   FMT   |       PT      |          length               |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                  SSRC of packet sender                        |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |             SSRC of media source (unused) = 0                 |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  :            Feedback Control Information (FCI)                 :
//  :                                                               :
// Full intra request (FIR) (RFC 5104).
// The Feedback Control Information (FCI) for the Full Intra Request
// consists of one or more FCI entries.
// FCI:
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                              SSRC                             |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  | Seq nr.       |    Reserved = 0                               |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// FMT Must be 4, PT must be PSFB
// RTPFB  |  205  | Transport layer FB message
// PSFB   |  206  | Payload-specific FB message
typedef struct {
    uint32_t media_ssrc;
    unsigned char seq;
    unsigned char reserved[3];
} RtcpPacketFIRItem; /* BIG END */

bool RtpSession::GobalInit() {
	bool bFlag = false;

    /* initialize srtp library */
	srtp_err_status_t status = srtp_init();
	bFlag = (status == srtp_err_status_ok);

	LogAync(
			LOG_ERR_USER,
			"RtpSession::GobalInit( "
			"[%s] "
			")",
			bFlag?"OK":"Fail"
			);

	return bFlag;
}

RtpSession::RtpSession():
		mClientMutex(KMutex::MutexType_Recursive) {
	// TODO Auto-generated constructor stub
	// Status
	mRunning = false;

	// Socket
	mpRtpSender = NULL;
	mpRtcpSender = NULL;

	// libsrtp
	mpSendSrtpCtx = NULL;
	mpRecvSrtpCtx = NULL;
	mpSendPolicy = new srtp_policy_t();
	mpRecvPolicy = new srtp_policy_t();

	Reset();
}

RtpSession::~RtpSession() {
	// TODO Auto-generated destructor stub
    if (mpSps) {
        delete[] mpSps;
        mpSps = NULL;
    }
    mSpsSize = 0;

    if (mpPps) {
        delete[] mpPps;
        mpPps = NULL;
    }
    mPpsSize = 0;

    if ( mpRecvPolicy ) {
    	delete mpRecvPolicy;
    	mpRecvPolicy = NULL;
    }

    if ( mpSendPolicy ) {
    	delete mpSendPolicy;
    	mpSendPolicy = NULL;
    }
}

void RtpSession::Reset() {
	// Video
	mVideoTimestamp = 0;
	mVideoRtpTimestamp = 0;
	mVideoRtpSeq = 0;
	mNaluHeaderSize = 0;
	mInputVideoTimestamp = 0;
	mSendVideoFrameTimestamp = 0;
	mVideoFrameCount = 0;

	// Audio
	mAudioTimestamp = 0;
	mAudioRtpTimestamp = 0;
	mAudioRtpSeq = 0;

	mpSps = NULL;
	mSpsSize = 0;
	mpPps = NULL;
	mPpsSize = 0;

	mFirSeq = 0;
	mVideoFrameCount = 0;
}

void RtpSession::SetRtpSender(SocketSender *sender) {
	mpRtpSender = sender;
}

void RtpSession::SetRtcpSender(SocketSender *sender) {
	mpRtcpSender = sender;
}

bool RtpSession::Start(char *localKey, int localSize, char *remoteKey, int remoteSize) {
	bool bFlag = false;

	Arithmetic art;
	LogAync(
			LOG_MSG,
			"RtpSession::Start( "
			"this : %p, "
			"localKey : %s, "
			"remoteKey : %s "
			")",
			this,
			art.AsciiToHexWithSep(localKey, localSize).c_str(),
			art.AsciiToHexWithSep(remoteKey, remoteSize).c_str()
			);

	srtp_err_status_t status = srtp_err_status_ok;

	mClientMutex.lock();
	if( mRunning ) {
		Stop();
	}

	mRunning = true;

	bFlag = StartSend(localKey, localSize);
	bFlag &= StartRecv(remoteKey, remoteSize);

	if( bFlag ) {
		LogAync(
				LOG_MSG,
				"RtpSession::Start( "
				"this : %p, "
				"[OK] "
				")",
				this
				);
	} else {
		LogAync(
				LOG_ERR_SYS,
				"RtpSession::Start( "
				"this : %p, "
				"[Fail], "
				"status : %d "
				")",
				this,
				status
				);
		Stop();
	}

	mClientMutex.unlock();

	return bFlag;
}

void RtpSession::Stop() {
	LogAync(
			LOG_MSG,
			"RtpSession::Stop( "
			"this : %p "
			")",
			this
			);

	mClientMutex.lock();
	if( mRunning ) {
		mRunning = false;

		StopRecv();
		StopSend();

	    if (mpSps) {
	        delete[] mpSps;
	        mpSps = NULL;
	    }
	    mSpsSize = 0;

	    if (mpPps) {
	        delete[] mpPps;
	        mpPps = NULL;
	    }
	    mPpsSize = 0;

		Reset();

//		LogAync(
//				LOG_MSG,
//				"RtpSession::Stop( "
//				"this : %p, "
//				"[OK] "
//				")",
//				this
//				);
	}
	mClientMutex.unlock();

	LogAync(
			LOG_MSG,
			"RtpSession::Stop( "
			"this : %p "
			"[OK] "
			")",
			this
			);
}

bool RtpSession::StartSend(char *localKey, int size) {
	bool bFlag = true;

	Arithmetic art;
	LogAync(
			LOG_STAT,
			"RtpSession::StartSend( "
			"this : %p, "
			"size : %d, "
			"localKey : %s "
			")",
			this,
			size,
			art.AsciiToHexWithSep(localKey, size).c_str()
			);

	// Initialize libsrtp
	memset(mpSendPolicy, 0x0, sizeof(srtp_policy_t));

	if( localKey && size > 0 ) {
		srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&mpSendPolicy->rtp);
		srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&mpSendPolicy->rtcp);
		mpSendPolicy->key = (uint8_t *)localKey;
	} else {
		srtp_crypto_policy_set_null_cipher_hmac_null(&mpSendPolicy->rtp);
		srtp_crypto_policy_set_null_cipher_hmac_null(&mpSendPolicy->rtcp);
		mpSendPolicy->key = (uint8_t *)kEmptyKey;
	}

	mpSendPolicy->ssrc.type = ssrc_any_outbound;
	mpSendPolicy->ssrc.value = 0;
	mpSendPolicy->window_size = 0;
	mpSendPolicy->allow_repeat_tx = 0;
	mpSendPolicy->ekt = NULL;
	mpSendPolicy->next = NULL;

    srtp_err_status_t status = srtp_err_status_ok;
	status = srtp_create(&mpSendSrtpCtx, mpSendPolicy);
	bFlag = (status == srtp_err_status_ok);

	if( bFlag ) {
		LogAync(
				LOG_STAT,
				"RtpSession::StartSend( "
				"this : %p, "
				"[OK] "
				")",
				this
				);
	} else {
		LogAync(
				LOG_ERR_SYS,
				"RtpSession::StartSend( "
				"this : %p, "
				"[Fail], "
				"status : %d "
				")",
				this,
				status
				);
	}

	return bFlag;
}

void RtpSession::StopSend() {
	if( mpSendSrtpCtx ) {
		srtp_dealloc(mpSendSrtpCtx);
		mpSendSrtpCtx = NULL;
	}

	LogAync(
			LOG_STAT,
			"RtpSession::StopSend( "
			"this : %p, "
			"[OK] "
			")",
			this
			);
}

bool RtpSession::StartRecv(char *remoteKey, int size) {
	bool bFlag = true;

	Arithmetic art;
	LogAync(
			LOG_STAT,
			"RtpSession::StartRecv( "
			"this : %p, "
			"size : %d, "
			"remoteKey : %s "
			")",
			this,
			size,
			art.AsciiToHexWithSep(remoteKey, size).c_str()
			);

	// Initialize libsrtp
	memset(mpRecvPolicy, 0x0, sizeof(srtp_policy_t));

	if( remoteKey && size > 0 ) {
		srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&mpRecvPolicy->rtp);
		srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&mpRecvPolicy->rtcp);
		mpRecvPolicy->key = (uint8_t *)remoteKey;

	} else {
		srtp_crypto_policy_set_null_cipher_hmac_null(&mpRecvPolicy->rtp);
		srtp_crypto_policy_set_null_cipher_hmac_null(&mpRecvPolicy->rtcp);
		mpRecvPolicy->key = (uint8_t *)kEmptyKey;
	}

    mpRecvPolicy->ssrc.type = ssrc_any_inbound;
    mpRecvPolicy->ssrc.value = 0;
    mpRecvPolicy->window_size = 0;
    mpRecvPolicy->allow_repeat_tx = 0;
    mpRecvPolicy->ekt = NULL;
    mpRecvPolicy->next = NULL;

    srtp_err_status_t status = srtp_err_status_ok;
	status = srtp_create(&mpRecvSrtpCtx, mpRecvPolicy);
	bFlag = (status == srtp_err_status_ok);

	if( bFlag ) {
		LogAync(
				LOG_STAT,
				"RtpSession::StartRecv( "
				"this : %p, "
				"[OK] "
				")",
				this
				);
	} else {
		LogAync(
				LOG_ERR_SYS,
				"RtpSession::StartRecv( "
				"this : %p, "
				"[Fail], "
				"status : %d "
				")",
				this,
				status
				);
	}

	return bFlag;
}

void RtpSession::StopRecv() {
	if( mpRecvSrtpCtx ) {
		srtp_dealloc(mpRecvSrtpCtx);
		mpRecvSrtpCtx = NULL;
	}

	LogAync(
			LOG_STAT,
			"RtpSession::StopRecv( "
			"this : %p, "
			"[OK] "
			")",
			this
			);
}

void RtpSession::SetVideoKeyFrameInfoH264(const char *sps, int spsSize, const char *pps, int ppsSize, int naluHeaderSize, u_int32_t timestamp) {
	mNaluHeaderSize = naluHeaderSize;

    bool bSpsChange = true;
    if (mpSps != NULL) {
        if (mSpsSize == spsSize) {
            if (0 != memcmp(mpSps, sps, spsSize)) {
                bSpsChange = true;

            } else {
                bSpsChange = false;
            }
        }
    }

    if (bSpsChange) {
        if (mpSps) {
            delete[] mpSps;
            mpSps = NULL;
        }

        mSpsSize = spsSize;
        mpSps = new char[mSpsSize];
        memcpy(mpSps, sps, mSpsSize);
    }

    bool bPpsChange = true;
    if (mpPps != NULL) {
        if (mPpsSize == ppsSize) {
            if (0 != memcmp(mpPps, pps, ppsSize)) {
                bPpsChange = true;

            } else {
                bPpsChange = false;
            }
        }
    }

    if (bPpsChange) {
        if (mpPps) {
            delete[] mpPps;
            mpPps = NULL;
        }

        mPpsSize = ppsSize;
        mpPps = new char[mPpsSize];
        memcpy(mpPps, pps, mPpsSize);
    }

    if( bSpsChange || bPpsChange ) {
    	LogAync(
    			LOG_STAT,
    			"RtpSession::SetVideoKeyFrameInfoH264( "
    			"this : %p, "
    			"timestamp : %u, "
				"mSpsSize : %d, "
    			"mPpsSize : %d, "
    			"mNaluHeaderSize : %d "
    			")",
    			this,
    			timestamp,
				mSpsSize,
				mPpsSize,
				mNaluHeaderSize
    			);
    }
}

bool RtpSession::SendVideoFrameH264(const char* frame, unsigned int size, unsigned int timestamp) {
	bool bFlag = true;

	srtp_err_status_t status = srtp_err_status_ok;

	mClientMutex.lock();
	if( mRunning ) {
        // 计算RTP时间戳
        int sendTimestamp = 0;

        // 第一帧
        if (mInputVideoTimestamp == 0) {
        	mInputVideoTimestamp = timestamp;
        }

        // 当前帧比上一帧时间戳大, 计算时间差
        if (timestamp > mInputVideoTimestamp) {
            sendTimestamp = timestamp - mInputVideoTimestamp;
        }

        mSendVideoFrameTimestamp += sendTimestamp * 90;
        mInputVideoTimestamp = timestamp;

        SendVideoKeyFrameH264();

		RtpPacket pkt;
		int lastSize = 0;
		int bodySize = 0;
		int pktSize = 0;

	    Nalu naluArray[16];
	    int naluArraySize = _countof(naluArray);
	    bool bFlag = mVideoMuxer.GetNalus(frame, size, mNaluHeaderSize, naluArray, naluArraySize);
	    if( bFlag && naluArraySize > 0 ) {
	        int naluIndex = 0;
	        while( naluIndex < naluArraySize ) {
	            Nalu* nalu = naluArray + naluIndex;
	            naluIndex++;

	        	const char *body = nalu->GetNaluBody();
	        	lastSize = nalu->GetNaluBodySize();
	        	if( lastSize <= sizeof(pkt.body) ) {
	        		// Send single NALU with one RTP packet
		        	LogAync(
		        			LOG_STAT,
		        			"RtpSession::SendVideoFrameH264( "
		        			"this : %p, "
							"[Send single NALU with single RTP packet], "
		        			"timestamp : %u, "
							"naluType : %d, "
							"naluBodySize : %d, "
							"[Mark] "
		        			")",
							this,
							mSendVideoFrameTimestamp,
							nalu->GetNaluType(),
							nalu->GetNaluBodySize()
		        			);

	    			pkt.header.ssrc = htonl(0x12345678);
	    			pkt.header.seq = htons(mVideoRtpSeq++);
	    			pkt.header.ts = htonl(mSendVideoFrameTimestamp);
	    			pkt.header.m = 1;
	    			pkt.header.pt = 96;
	    			pkt.header.version = 2;
	    			pkt.header.p = 0;
	    			pkt.header.x = 0;
	    			pkt.header.cc = 0;

	    			bodySize = lastSize;
	    			// NALU payload
	    			memcpy(pkt.body, body, bodySize);

	    			pktSize = RTP_HEADER_LEN + bodySize;
	    			status = srtp_protect(mpSendSrtpCtx, &pkt.header, &pktSize);
	    		    if (status == srtp_err_status_ok) {
	    		    	if( mpRtpSender ) {
	    		    		int sendSize = mpRtpSender->SendData((void *)&pkt, pktSize);
							if (sendSize != pktSize) {
								bFlag = false;
							}
	    		    	} else {
	    		    		bFlag = false;
	    		    	}
	    		    } else {
	    		    	bFlag = false;
	    		    }

	        	} else {
	        		// Send single NALU with multiple RTP packets
		        	LogAync(
		        			LOG_STAT,
		        			"RtpSession::SendVideoFrameH264( "
		        			"this : %p, "
							"[Send single NALU with multiple RTP packets], "
		        			"timestamp : %u, "
							"naluType : %d, "
							"naluBodySize: %d "
		        			")",
							this,
							mSendVideoFrameTimestamp,
							nalu->GetNaluType(),
							nalu->GetNaluBodySize()
		        			);

		        	// Wipe off original NALU header
		        	char naluHeader = body[0];

		            Slice* sliceArray = NULL;
		            int sliceArraySize = 0;
		            int sliceIndex = 0;

					nalu->GetSlices(&sliceArray, sliceArraySize);
					while( sliceIndex < sliceArraySize ) {
						Slice* slice = sliceArray + sliceIndex;
						sliceIndex++;

			        	const char *body = slice->GetSlice();
			        	lastSize = slice->GetSliceSize();
			        	lastSize--;
			        	body++;
			        	int packetIndex = 0;
			        	int sentSize = 0;

			        	while( true ) {
			        		// 2 bytes for FU indicator and FU header
			        		bodySize = MIN(lastSize, sizeof(pkt.body) - 2);

			    			pkt.header.ssrc = htonl(0x12345678);
			    			pkt.header.seq = htons(mVideoRtpSeq++);
			    			pkt.header.ts = htonl(mSendVideoFrameTimestamp);
			    			pkt.header.m = (lastSize <= bodySize)?1:0;
			    			pkt.header.pt = 96;
			    			pkt.header.version = 2;
			    			pkt.header.p = 0;
			    			pkt.header.x = 0;
			    			pkt.header.cc = 0;

		    		    	// Hard code here, cause 0x7C means important, 0x5C means not
		    		    	/**
		    		    	 * FU indicator
		    		    	 * Hard code here, cause 0x7C means important, 0x5C means not
		    		    	 * [F].[NRI].[  TYPE   ]
		    		    	 * [7].[6.5].[4.3.2.1.0]
		    		    	 * 28 : FU-A
		    		    	 */
		    		    	pkt.body[0] = 0x60 | 28;
		    		    	/**
		    		    	 * FU header
		    		    	 * [S].[E].[R].[  TYPE   ]
		    		    	 * [7].[6].[5].[4.3.2.1.0]
		    		    	 */
							pkt.body[1] = nalu->GetNaluType();
		    		    	if( packetIndex == 0 && !pkt.header.m ) {
		    		    		// Start packet
		    		    		pkt.body[1] |= 0x80;
		    		    	} else if( pkt.header.m ) {
		    		    		// Last packet
		    		    		pkt.body[1] |= 0x40;
		    		    	} else {
		    		    		// Middle packet
		    		    	}

		    		    	// FU payload
							memcpy(pkt.body + 2, body + sentSize, bodySize);

			    			// Whole size
			    			pktSize = RTP_HEADER_LEN + 2 + bodySize;
			    			status = srtp_protect(mpSendSrtpCtx, &pkt.header, &pktSize);
			    		    if (status == srtp_err_status_ok) {
			    		    	int sendSize = 0;
			    		    	if( mpRtpSender ) {
			    		    		sendSize = mpRtpSender->SendData((void *)&pkt, pktSize);
			    		    	}

								unsigned char t = nalu->GetNaluType();
								unsigned char d = pkt.body[0];
								unsigned char h = pkt.body[1];

					        	LogAync(
					        			LOG_STAT,
										"RtpSession::SendVideoFrameH264( "
										"this : %p, "
										"[Send FUs packet], "
										"timestamp : %u, "
										"NALU-T : 0x%02x, "
										"FU-D : 0x%02x, "
										"FU-H : 0x%02x, "
										"pktSize : %d "
										"%s"
										")",
										this,
										mSendVideoFrameTimestamp,
										t,
										d,
										h,
										pktSize,
										pkt.header.m?", [Mark] ":""
										);

								if (sendSize == pktSize) {
									sentSize += bodySize;
									lastSize -= bodySize;
									packetIndex++;

									if( lastSize == 0 ) {
										// All data has been sent
										break;
									}
								} else {
									bFlag = false;
									break;
								}

			    		    } else {
			    		    	bFlag = false;
			    		    }
			        	}
					}
	        	}
	        }
	    } else {
	    	bFlag = false;
	    }
	} else {
		bFlag = false;
	}
	mClientMutex.unlock();

	return bFlag;
}

bool RtpSession::SendVideoKeyFrameH264() {
	bool bFlag = true;

	srtp_err_status_t status = srtp_err_status_ok;
	RtpPacket pkt;
	int lastSize = 0;
	int bodySize = 0;
	int pktSize = 0;

	mClientMutex.lock();
	if( mRunning ) {
		if( mpSps && mPpsSize && mVideoFrameCount++ % 10 == 0 ) {
	    	LogAync(
	    			LOG_STAT,
	    			"RtpSession::SendVideoKeyFrameH264( "
	    			"this : %p, "
					"[Send SPS/PPS RTP packet], "
	    			"timestamp : %u "
	    			")",
					this,
					mSendVideoFrameTimestamp
	    			);

			pkt.header.ssrc = htonl(0x12345678);
			pkt.header.seq = htons(mVideoRtpSeq++);
			pkt.header.ts = htonl(mSendVideoFrameTimestamp);
			pkt.header.m = 0;
			pkt.header.pt = 96;
			pkt.header.version = 2;
			pkt.header.p = 0;
			pkt.header.x = 0;
			pkt.header.cc = 0;

	    	// Hard code here, cause 0x7C means important, 0x5C means not
	    	/**
	    	 * RTP payload header
	    	 * [F].[NRI].[  TYPE   ]
	    	 * [7].[6.5].[4.3.2.1.0]
	    	 * 24(0x18) : STAP-A
	    	 */
			bodySize = 0;
			pkt.body[bodySize++] = 0x18;
			// sps size 2 bytes
			short spsLength = htons(mSpsSize);
			memcpy(pkt.body + bodySize, (const void *)&spsLength, mSpsSize);
			bodySize += sizeof(spsLength);
			// sps data
			memcpy(pkt.body + bodySize, mpSps, mSpsSize);
			bodySize += mSpsSize;

			// pps size 2 bytes
			short ppsLength = htons(mPpsSize);
			memcpy(pkt.body + bodySize, (const void *)&ppsLength, sizeof(ppsLength));
			bodySize += sizeof(ppsLength);
			// pps data
			memcpy(pkt.body + bodySize, mpPps, mPpsSize);
			bodySize += mPpsSize;

			pktSize = RTP_HEADER_LEN + bodySize;
			status = srtp_protect(mpSendSrtpCtx, &pkt.header, &pktSize);
		    if (status == srtp_err_status_ok) {
		    	if( mpRtpSender ) {
		    		int sendSize = mpRtpSender->SendData((void *)&pkt, pktSize);
					if (sendSize != pktSize) {
						bFlag = false;
					}
		    	} else {
		    		bFlag = false;
		    	}
		    } else {
		    	bFlag = false;
		    }
		}

	}
	
	mClientMutex.unlock();
	return bFlag;
}

bool RtpSession::SendAudioFrame(const char* frame, unsigned int size, unsigned int timestamp) {
	bool bFlag = false;

	LogAync(
			LOG_WARNING,
			"RtpSession::SendAudioFrame( "
			"this : %p, "
			"timestamp : %d, "
			"size : %u "
			")",
			this,
			timestamp,
			size
			);

	return bFlag;
}


bool RtpSession::SendRtpPacket(void *pkt, unsigned int& pktSize) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;
	mClientMutex.lock();
	if( mRunning ) {
		status = srtp_protect(mpSendSrtpCtx, pkt, (int *)&pktSize);
	}
	mClientMutex.unlock();

    if (status == srtp_err_status_ok) {
    	if( mpRtpSender ) {
    		int sendSize = mpRtpSender->SendData((void *)pkt, pktSize);
			if (sendSize != pktSize) {
				bFlag = false;
			}
    	} else {
    		bFlag = false;
    	}
    } else {
    	bFlag = false;
    }

//	LogAync(
//			LOG_MSG,
//			"RtpSession::SendRtpPacket( "
//			"this : %p, "
//			"status : %d, "
//			"pktSize : %d, "
//			"seq : %u, "
//			"timestamp : %u "
//			")",
//			this,
//			status,
//			pktSize,
//			ntohs(pkt->header.seq),
//			ntohl(pkt->header.ts)
//			);

	return bFlag;
}

bool RtpSession::RecvRtpPacket(const char* frame, unsigned int size, void *pkt, unsigned int& pktSize) {
	bool bFlag = false;

	mClientMutex.lock();
	if( mRunning ) {
		srtp_err_status_t status = srtp_err_status_ok;
		if( IsRtp(frame, size) ) {
			memcpy((void *)pkt, (void *)frame, size);
			pktSize = size;

			if ( mpRecvSrtpCtx ) {
				srtp_err_status_t status = srtp_unprotect(mpRecvSrtpCtx, pkt, (int *)&pktSize);
				bFlag = (status == srtp_err_status_ok);

				if( bFlag ) {
					if( ((RtpPacket *)pkt)->header.m ) {
						// 每10个视频帧强制刷新一次视频信息
						if( ++mVideoFrameCount % 15 == 0 ) {
							SendRtcpFIR(((RtpPacket *)pkt)->header.ssrc);
						}
					}
				}
			}
	//		LogAync(
	//				LOG_MSG,
	//				"RtpSession::RecvRtpPacket( "
	//				"this : %p, "
	//				"status : %d, "
	//				"seq : %u, "
	//				"timestamp : %u, "
	//				"pktSize : %d "
	//				"%s"
	//				")",
	//				this,
	//				status,
	//				ntohs(pkt->header.seq),
	//				ntohl(pkt->header.ts),
	//				pktSize,
	//				pkt.header.m?", [Mark] ":""
	//				);
		} else {
			LogAync(
					LOG_WARNING,
					"RtpSession::RecvRtpPacket( "
					"this : %p, "
					"[Ignore frame before Handshake] "
					")",
					this
					);
		}
	}
	mClientMutex.unlock();

	return bFlag;
}

bool RtpSession::SendRtcpPacket(void *pkt, unsigned int& pktSize) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;
	mClientMutex.lock();
	if( mRunning ) {
		status = srtp_protect_rtcp(mpSendSrtpCtx, pkt, (int *)&pktSize);
	}
	mClientMutex.unlock();

    if (status == srtp_err_status_ok) {
    	if( mpRtcpSender ) {
    		int sendSize = mpRtcpSender->SendData((void *)pkt, pktSize);
			if (sendSize != pktSize) {
				bFlag = false;
			}
    	} else {
    		bFlag = false;
    	}
    } else {
    	bFlag = false;
    }

	return bFlag;
}

bool RtpSession::RecvRtcpPacket(const char* frame, unsigned int size, void *pkt, unsigned int& pktSize) {
	bool bFlag = false;

	mClientMutex.lock();
	if( mRunning ) {
		srtp_err_status_t status = srtp_err_status_ok;
		if( IsRtcp(frame, size) ) {
			memcpy((void *)pkt, (void *)frame, size);
			pktSize = size;

			if ( mpRecvSrtpCtx ) {
				srtp_err_status_t status = srtp_unprotect_rtcp(mpRecvSrtpCtx, pkt, (int *)&pktSize);
				bFlag = (status == srtp_err_status_ok);
			}

		} else {
			LogAync(
					LOG_WARNING,
					"RtpSession::RecvRtcpPacket( "
					"this : %p, "
					"[Ignore frame before Handshake] "
					")",
					this
					);
		}
	}
	mClientMutex.unlock();

	return bFlag;
}

bool RtpSession::SendRtcpPLI(unsigned int remoteSSRC) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;
	RtcpPacketPLI pkt = {0};
	int pktSize = 0;

	Arithmetic art;
	mClientMutex.lock();
	if( mRunning ) {
		pkt.header.version = 2;
		pkt.header.p = 0;
		pkt.header.rc = 1;
		pkt.header.pt = 206;
		pkt.header.length = 2;
		pkt.header.length = htons(pkt.header.length);
		pkt.ssrc = htonl(0x12345678);
		pkt.media_ssrc = remoteSSRC;

		pktSize = sizeof(RtcpPacketPLI);

		status = srtp_protect_rtcp(mpSendSrtpCtx, (void *)&pkt, &pktSize);
	}
	mClientMutex.unlock();

    if (status == srtp_err_status_ok) {
    	if( mpRtcpSender ) {
    		int sendSize = mpRtcpSender->SendData((void *)&pkt, pktSize);
			if (sendSize != pktSize) {
				bFlag = false;
			}
    	} else {
    		bFlag = false;
    	}
    }

	return bFlag;
}

bool RtpSession::SendRtcpFIR(unsigned int remoteSSRC) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;
	char tmp[MTU];
	int pktSize = 0;

	mClientMutex.lock();
	if( mRunning ) {
		RtcpPacketPLI pkt = {0};
		pkt.header.version = 2;
		pkt.header.p = 0;
		pkt.header.rc = 4;
		pkt.header.pt = 206;
		pkt.header.length = 2 + 2;
		pkt.header.length = htons(pkt.header.length);
		pkt.ssrc = htonl(0x12345678);
		pkt.media_ssrc = remoteSSRC;

		RtcpPacketFIRItem item = {0};
		item.media_ssrc = remoteSSRC;
		item.seq = ++mFirSeq;

		memcpy(tmp, (void *)&pkt, sizeof(RtcpPacketPLI));
		pktSize += sizeof(RtcpPacketPLI);
		memcpy(tmp + pktSize, (void *)&item, sizeof(RtcpPacketFIRItem));
		pktSize += sizeof(RtcpPacketFIRItem);

		status = srtp_protect_rtcp(mpSendSrtpCtx, (void *)tmp, &pktSize);
	}
	mClientMutex.unlock();

    if (status == srtp_err_status_ok) {
		if( mpRtcpSender ) {
			int sendSize = mpRtcpSender->SendData((void *)tmp, pktSize);
			if (sendSize != pktSize) {
				bFlag = false;
			}
		} else {
			bFlag = false;
		}
    }

	return bFlag;
}

unsigned int RtpSession::GetRtpSSRC(void *pkt, unsigned int& pktSize) {
	int ssrc = 0;
	RtpHeader *header = (RtpHeader *)pkt;
	ssrc = ntohl(header->ssrc);
	return ssrc;
}

unsigned int RtpSession::GetRtcpSSRC(void *pkt, unsigned int& pktSize) {
	int ssrc = 0;
	RtcpPacketCommon *header = (RtcpPacketCommon *)pkt;
	ssrc = ntohl(header->ssrc);
	return ssrc;
}

bool RtpSession::IsRtp(const char *frame, unsigned len) {
	bool bFlag = false;
	if( len > 0 ) {
		RtpHeader *header = (RtpHeader *)frame;
		bFlag = ((header->pt < 64) || ((header->pt >= 96) && (header->pt < 200)));
	}
	return bFlag;
}

bool RtpSession::IsRtcp(const char *frame, unsigned len) {
	bool bFlag = false;
	if( len > 0 ) {
		RtcpHeader *header = (RtcpHeader *)frame;
		bFlag = ((header->pt >= 200) && (header->pt <= 210));
	}
	return bFlag;
}

} /* namespace mediaserver */
