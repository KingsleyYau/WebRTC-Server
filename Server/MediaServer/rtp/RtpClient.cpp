/*
 * RtpClient.cpp
 *
 *  Created on: 2019/06/20
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "RtpClient.h"

// Common
#include <common/LogManager.h>
#include <common/Math.h>
#include <common/CommonFunc.h>
#include <common/Arithmetic.h>

// libsrtp
#include <srtp.h>
#include <srtp_priv.h>

/*
 * SRTP key size
 */
#define MAX_KEY_LEN 96

//#define MTU 1500
///*
// * RTP_HEADER_LEN indicates the size of an RTP header
// */
//#define RTP_HEADER_LEN 12
//#define RTP_HEADER_SIZE 54
//#define RTP_MAX_BUF_LEN (MTU - RTP_HEADER_SIZE)
//typedef struct {
//    srtp_hdr_t header;
//    char body[RTP_MAX_BUF_LEN];
//} RtpMsg;

namespace mediaserver {
bool RtpClient::GobalInit() {
	bool bFlag = false;

    /* initialize srtp library */
	srtp_err_status_t status = srtp_init();
	bFlag = (status == srtp_err_status_ok);

	LogAync(
			LOG_ERR_USER,
			"RtpClient::GobalInit( "
			"[%s] "
			")",
			bFlag?"OK":"Fail"
			);

	return bFlag;
}

RtpClient::RtpClient():
		mClientMutex(KMutex::MutexType_Recursive) {
	// TODO Auto-generated constructor stub
	// Status
	mRunning = false;

	// Socket
	mpSocketSender = NULL;

	// libsrtp
	mpRecvPolicy = new srtp_policy_t();

	Reset();
}

RtpClient::~RtpClient() {
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
}

void RtpClient::Reset() {
	// libsrtp
	mpSrtpCtx = NULL;

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

	mRecving = false;
}

void RtpClient::SetSocketSender(SocketSender *sender) {
	mpSocketSender = sender;
}

bool RtpClient::Start() {
	bool bFlag = true;

	LogAync(
			LOG_MSG,
			"RtpClient::Start( "
			"this : %p, "
			")",
			this
			);

	srtp_err_status_t status = srtp_err_status_ok;

	mClientMutex.lock();
	if( mRunning ) {
		Stop();
	}

	mRunning = true;

    if( bFlag ) {
    	// Initialize libsrtp
    	srtp_policy_t policy;
    	memset(&policy, 0x0, sizeof(srtp_policy_t));

        /*
         * we're not providing security services, so set the policy to the
         * null policy
         *
         * Note that this policy does not conform to the SRTP
         * specification, since RTCP authentication is required.  However,
         * the effect of this policy is to turn off SRTP, so that this
         * application is now a vanilla-flavored RTP application.
         */
    	char key[MAX_KEY_LEN] = {0};

        srtp_crypto_policy_set_null_cipher_hmac_null(&policy.rtp);
        srtp_crypto_policy_set_null_cipher_hmac_null(&policy.rtcp);

        policy.key = (uint8_t *)key;
        policy.ssrc.type = ssrc_any_outbound;
        policy.window_size = 0;
        policy.allow_repeat_tx = 0;
        policy.ekt = NULL;
        policy.next = NULL;

    	status = srtp_create(&mpSrtpCtx, &policy);
    	bFlag = (status == srtp_err_status_ok);
    }

	if( bFlag ) {
		LogAync(
				LOG_MSG,
				"RtpClient::Start( "
				"this : %p, "
				"[OK] "
				")",
				this
				);
	} else {
		LogAync(
				LOG_ERR_SYS,
				"RtpClient::Start( "
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

void RtpClient::Stop() {
	mClientMutex.lock();
	if( mRunning ) {
		mRunning = false;

		if( mpSrtpCtx ) {
			srtp_dealloc(mpSrtpCtx);
			mpSrtpCtx = NULL;
		}

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
	}
	mClientMutex.unlock();

	LogAync(
			LOG_WARNING,
			"RtpClient::Stop( "
			"this : %p, "
			"[OK] "
			")",
			this
			);
}

bool RtpClient::StartRecv(char *remoteKey, int size) {
	bool bFlag = true;

	Arithmetic art;
	LogAync(
			LOG_MSG,
			"RtpClient::StartRecv( "
			"this : %p, "
			"size : %d, "
			"remoteKey : %s "
			")",
			this,
			size,
			art.AsciiToHexWithSep(remoteKey, size).c_str()
			);

	mClientMutex.lock();
	if( mRecving ) {
	}

	mRecving = true;

	// Initialize libsrtp
	memset(mpRecvPolicy, 0x0, sizeof(srtp_policy_t));

	srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&mpRecvPolicy->rtp);
	srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&mpRecvPolicy->rtcp);

    mpRecvPolicy->key = (uint8_t *)remoteKey;
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
				LOG_MSG,
				"RtpClient::StartRecv( "
				"this : %p, "
				"[OK] "
				")",
				this
				);
	} else {
		LogAync(
				LOG_ERR_SYS,
				"RtpClient::StartRecv( "
				"this : %p, "
				"[Fail], "
				"status : %d "
				")",
				this,
				status
				);
	}

	mClientMutex.unlock();

	return bFlag;
}

void RtpClient::SetVideoKeyFrameInfoH264(const char *sps, int spsSize, const char *pps, int ppsSize, int naluHeaderSize, u_int32_t timestamp) {
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
    			"RtpClient::SetVideoKeyFrameInfoH264( "
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

bool RtpClient::SendVideoFrameH264(const char* frame, unsigned int size, unsigned int timestamp) {
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
		        			"RtpClient::SendVideoFrameH264( "
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
	    			status = srtp_protect(mpSrtpCtx, &pkt.header, &pktSize);
	    		    if (status == srtp_err_status_ok) {
	    		    	if( mpSocketSender ) {
	    		    		int sendSize = mpSocketSender->SendData((void *)&pkt, pktSize);
							if (sendSize != pktSize) {
								bFlag = false;
							}
	    		    	} else {
	    		    		bFlag = false;
	    		    	}
//						int sendSize = sendto(mFd, (void *)&pkt, pktSize, 0, (struct sockaddr *)&mSendSockAddr, sizeof(struct sockaddr_in));
//						if (sendSize != pktSize) {
//							bFlag = false;
//						}
	    		    } else {
	    		    	bFlag = false;
	    		    }

	        	} else {
	        		// Send single NALU with multiple RTP packets
		        	LogAync(
		        			LOG_STAT,
		        			"RtpClient::SendVideoFrameH264( "
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
			    			status = srtp_protect(mpSrtpCtx, &pkt.header, &pktSize);
			    		    if (status == srtp_err_status_ok) {
			    		    	int sendSize = 0;
			    		    	if( mpSocketSender ) {
			    		    		sendSize = mpSocketSender->SendData((void *)&pkt, pktSize);
			    		    	}
//								int sendSize = sendto(mFd, (void *)&pkt, pktSize, 0, (struct sockaddr *)&mSendSockAddr, sizeof(struct sockaddr_in));

								unsigned char t = nalu->GetNaluType();
								unsigned char d = pkt.body[0];
								unsigned char h = pkt.body[1];

					        	LogAync(
					        			LOG_STAT,
										"RtpClient::SendVideoFrameH264( "
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

//	LogAync(
//			LOG_WARNING,
//			"RtpClient::SendVideoFrameH264( "
//			"this : %p, "
//			"[%s], "
//			"timestamp : %d, "
//			"size : %d, "
//			"status : %d "
//			")",
//			this,
//			bFlag?"Success":"Fail",
//			timestamp,
//			size,
//			status
//			);

	return bFlag;
}

bool RtpClient::SendVideoKeyFrameH264() {
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
	    			"RtpClient::SendVideoKeyFrameH264( "
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
			status = srtp_protect(mpSrtpCtx, &pkt.header, &pktSize);
		    if (status == srtp_err_status_ok) {
		    	if( mpSocketSender ) {
		    		int sendSize = mpSocketSender->SendData((void *)&pkt, pktSize);
					if (sendSize != pktSize) {
						bFlag = false;
					}
		    	} else {
		    		bFlag = false;
		    	}
//				int sendSize = sendto(mFd, (void *)&pkt, pktSize, 0, (struct sockaddr *)&mSendSockAddr, sizeof(struct sockaddr_in));
//				if (sendSize != pktSize) {
//					bFlag = false;
//				}
		    } else {
		    	bFlag = false;
		    }
		}

	}
	return bFlag;
}

bool RtpClient::SendAudioFrame(const char* frame, unsigned int size, unsigned int timestamp) {
	bool bFlag = false;

	LogAync(
			LOG_WARNING,
			"RtpClient::SendAudioFrame( "
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


bool RtpClient::SendRtpPacket(RtpPacket *pkt, unsigned int& pktSize) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_ok;

	mClientMutex.lock();
	if( mRunning ) {
		status = srtp_protect(mpSrtpCtx, pkt, (int *)&pktSize);
	    if (status == srtp_err_status_ok) {
	    	if( mpSocketSender ) {
	    		int sendSize = mpSocketSender->SendData((void *)pkt, pktSize);
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
	mClientMutex.unlock();

//	LogAync(
//			LOG_WARNING,
//			"RtpClient::SendRtpPacket( "
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

bool RtpClient::RecvRtpPacket(const char* frame, unsigned int size, RtpPacket *pkt, unsigned int& pktSize) {
	bool bFlag = false;

	Arithmetic art;
	srtp_err_status_t status = srtp_err_status_ok;
	if( mRecving && IsRtp(frame, size) ) {
		memcpy((void *)pkt, (void *)frame, size);
		pktSize = size;

//		LogAync(
//				LOG_WARNING,
//				"RtpClient::RecvRtpPacket( "
//				"this : %p, "
//				"size : %u, "
//				"seq : %u, "
//				"timestamp : %u, "
//				"protected : %s "
//				")",
//				this,
//				size,
//				ntohs(pkt.header.seq),
//				ntohl(pkt.header.ts),
//				art.AsciiToHexWithSep(pkt.body, size).c_str()
//				);

		srtp_err_status_t status = srtp_unprotect(mpRecvSrtpCtx, pkt, (int *)&pktSize);
		bFlag = (status == srtp_err_status_ok);

//		if( !bFlag ) {
			LogAync(
					LOG_WARNING,
					"RtpClient::RecvRtpPacket( "
					"this : %p, "
//					"[Fail]"
					"status : %d, "
					"seq : %u, "
					"timestamp : %u, "
					"pktSize : %d "
//					"body : %s "
					")",
					this,
					status,
					ntohs(pkt->header.seq),
					ntohl(pkt->header.ts),
					pktSize
//					art.AsciiToHexWithSep(pkt->body, pktSize - RTP_HEADER_LEN).c_str()
					);
//		}

	} else {
		LogAync(
				LOG_WARNING,
				"RtpClient::RecvRtpPacket( "
				"this : %p, "
				"[Ignore frame before Handshake] "
				")",
				this
				);
	}

	return bFlag;
}

bool RtpClient::IsRtp(const char *frame, unsigned len) {
	bool bFlag = false;
	if( len > 0 ) {
		srtp_hdr_t *header = (srtp_hdr_t *)frame;
		bFlag = ((header->pt < 64) || (header->pt >= 96));
	}
	return bFlag;
}

} /* namespace mediaserver */
