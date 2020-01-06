/*
 * RtpSession.cpp
 *
 *  Created on: 2019/06/20
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include <include/CommonHeader.h>

#include <common/LogManager.h>
#include <common/Math.h>
#include <common/CommonFunc.h>
#include <common/Arithmetic.h>
#include <common/Math.h>

#include "RealTimeClock.h"

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

#define RTCP_SSRC 0x12345678

#pragma pack(push, 1)
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

/**
 * profile = 0xBEDE, One byte header
	0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |  ID   | len=2 |              data				              |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

 * profile = 0x1x00, Two byte header
 	0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |  ID   		 | 		len=2    |             data		          |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

 */
typedef struct {
	uint16_t profile;          /* extension header profile  */
    uint16_t length;           /* extension header length   */
} RtpExtHeader; /* BIG END */

/**
 * Transmission Time Offsets in RTP Streams
 * https://tools.ietf.org/html/rfc5450
   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |  ID   | len=2 |              transmission offset              |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct {
	unsigned char id : 4;       /* id  */
	unsigned char len : 4;      /* len = 2  */
    uint32_t trans_offset : 24;	/* transmission offset */
} RtpExtTransOffset; /* BIG END */

enum RtcpPayloadType {
	RtcpPayloadTypeSR = 200,
	RtcpPayloadTypeRR = 201,
	RtcpPayloadTypeRTPFB = 205,	// Transport layer FB message
	RtcpPayloadTypePSFB = 206, 	// Payload-specific FB message
};

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
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |V=2|P|   FMT   |       PT      |          length               |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                  SSRC of packet sender                        |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                  SSRC of media source                         |
typedef struct {
	RtcpHeader header;
	uint32_t ssrc;
} RtcpPacketCommon; /* BIG END */

typedef struct {
	RtcpPacketCommon common;
	uint32_t ntp_ts_most;
	uint32_t ntp_ts_least;
    uint32_t rtp_ts;
    uint32_t sender_packet_count;
    uint32_t sender_octet_count;
} RtcpPacketSR; /* BIG END */

typedef struct {
	uint32_t media_ssrc;
    unsigned char fraction;     /* fraction lost	 							*/
    uint32_t packet_lost : 24;	/* cumulative packet lost 						*/
    uint32_t max_seq;			/* extended highest sequence number received	*/
    uint32_t jitter;			/* jitter		 								*/
    uint32_t lsr;				/* last SR		 								*/
    uint32_t dlsr;				/* delay last SR		 						*/
} RtcpPacketRR; /* BIG END */

// RFC 4585: Feedback format.
//
// PLI packet format:
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
// PSFB   |  206  | Payload-specific FB message
typedef struct {
	RtcpHeader header;
    uint32_t ssrc;
    uint32_t media_ssrc;
} RtcpPacketPLI; /* BIG END */

// RFC 4585: Feedback format.
//
// FIR packet format:
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
// PSFB   |  206  | Payload-specific FB message
typedef struct {
    uint32_t media_ssrc;
    unsigned char seq;
    unsigned char reserved[3];
} RtcpPacketFIRItem; /* BIG END */

// RFC 4585: Feedback format.
//
// Nack packet format:
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |V=2|P|   FMT   |       PT      |          length               |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                  SSRC of packet sender                        |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                  SSRC of media source                         |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  :        Packet Identifier 		|	Bitmap Lost Packet		    :
//  :                                                               :
// FMT Must be 1, PT must be PSFB
// RTPFB  |  205  | Transport layer FB message
typedef struct {
	uint16_t seq;
    uint16_t bitmap;
} RtcpPacketNackItem; /* BIG END */
#pragma pack(pop)

bool RtpSession::GobalInit() {
	bool bFlag = false;

    /* initialize srtp library */
	srtp_err_status_t status = srtp_init();
	bFlag = (status == srtp_err_status_ok);

	if ( !bFlag ) {
		LogAync(
				LOG_ALERT,
				"RtpSession::GobalInit( "
				"[%s] "
				")",
				FLAG_2_STRING(bFlag)
				);
	}

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

	//////////////////////////////////////////////////////////////////////////
	/**
	 * 模拟丢包
	 */
	mbSimLost = false;
	mAudioLostSeed = 300;
	mAudioLostSize = 60;
	mVideoLostSeed = 300;
	mVideoLostSize = 60;
    //////////////////////////////////////////////////////////////////////////

	Reset();
}

RtpSession::~RtpSession() {
	// TODO Auto-generated destructor stub
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
	mVideoMaxTimestamp = 0;
	mVideoMaxSeq = 0;
	mVideoSSRC = 0;

	mVideoTotalRecvPacket = 0;
	mVideoLastMaxSeq = 0;
	mVideoLastTotalRecvPacket = 0;

	mFirSeq = 0;
	mVideoFrameCount = 0;

	// Audio
	mAudioMaxTimestamp = 0;
	mAudioMaxSeq = 0;
	mAudioSSRC = 0;

	mAudioTotalRecvPacket = 0;
	mAudioLastMaxSeq = 0;
	mAudioLastTotalRecvPacket = 0;

	//////////////////////////////////////////////////////////////////////////
	/**
	 * 模拟丢包
	 */
	mbAudioAbandonning = false;
	mAudioAbandonTotal = 0;
	mAudioAbandonCount = 0;
	mbVideoAbandonning = false;
    mVideoAbandonTotal = 0;
    mVideoAbandonCount = 0;
    //////////////////////////////////////////////////////////////////////////
}

void RtpSession::SetRtpSender(SocketSender *sender) {
	mpRtpSender = sender;
}

void RtpSession::SetRtcpSender(SocketSender *sender) {
	mpRtcpSender = sender;
}

void RtpSession::SetVideoSSRC(unsigned int ssrc) {
	mVideoSSRC = ssrc;
}

void RtpSession::SetAudioSSRC(unsigned int ssrc) {
	mAudioSSRC = ssrc;
}

bool RtpSession::Start(char *localKey, int localSize, char *remoteKey, int remoteSize) {
	bool bFlag = false;

	Arithmetic art;
	LogAync(
			LOG_INFO,
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
				LOG_INFO,
				"RtpSession::Start( "
				"this : %p, "
				"[OK] "
				")",
				this
				);
	} else {
		LogAync(
				LOG_ALERT,
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
			LOG_INFO,
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

//		LogAync(
//				LOG_INFO,
//				"RtpSession::Stop( "
//				"this : %p, "
//				"[OK] "
//				")",
//				this
//				);
	}
	mClientMutex.unlock();

	LogAync(
			LOG_INFO,
			"RtpSession::Stop( "
			"this : %p "
			"[OK] "
			")",
			this
			);
}

void RtpSession::SetSimLostParam(
		 bool bSimLost,
		 unsigned int audioLostSeed,
		 unsigned int audioLostSize,
		 unsigned int videoLostSeed,
		 unsigned int videoLostSize
		 ) {
	mbSimLost = bSimLost;
	mAudioLostSeed = audioLostSeed;
	mAudioLostSize = audioLostSize;
	mVideoLostSeed = videoLostSeed;
	mVideoLostSize = videoLostSize;
}

bool RtpSession::StartSend(char *localKey, int size) {
	bool bFlag = true;

	Arithmetic art;
	LogAync(
			LOG_DEBUG,
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
				LOG_DEBUG,
				"RtpSession::StartSend( "
				"this : %p, "
				"[OK] "
				")",
				this
				);
	} else {
		LogAync(
				LOG_ALERT,
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
			LOG_DEBUG,
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
			LOG_DEBUG,
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
				LOG_DEBUG,
				"RtpSession::StartRecv( "
				"this : %p, "
				"[OK] "
				")",
				this
				);
	} else {
		LogAync(
				LOG_ALERT,
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
			LOG_DEBUG,
			"RtpSession::StopRecv( "
			"this : %p, "
			"[OK] "
			")",
			this
			);
}

bool RtpSession::RecvRtpPacket(const char* frame, unsigned int size, void *pkt, unsigned int& pktSize) {
	bool bFlag = false;

	mClientMutex.lock();
	if( mRunning ) {
		srtp_err_status_t status = srtp_err_status_ok;
		if( IsRtp(frame, size) ) {
			memcpy((void *)pkt, (void *)frame, size);
			pktSize = size;

			unsigned short seq = ntohs(((RtpPacket *)pkt)->header.seq);
			uint32_t ts = ntohl(((RtpPacket *)pkt)->header.ts);
			uint32_t ssrc = ntohl(((RtpPacket *)pkt)->header.ssrc);

			LogAync(
					LOG_DEBUG,
					"RtpSession::RecvRtpPacket( "
					"this : %p, "
					"[%s], "
					"ssrc : 0x%08x(%u), "
					"x : %u, "
					"cc : %u, "
					"seq : %u, "
					"timestamp : %u, "
					"pktSize : %d"
					"%s"
					")",
					this,
					PktTypeDesc(ssrc).c_str(),
					ssrc,
					ssrc,
					((RtpPacket *)pkt)->header.x,
					((RtpPacket *)pkt)->header.cc,
					seq,
					ts,
					pktSize,
					((RtpPacket *)pkt)->header.m?", [Mark] ":" "
					);

			if ( ((RtpPacket *)pkt)->header.x ) {
				unsigned int payloadOffset = sizeof(RtpHeader) + (((RtpHeader *)pkt)->cc * 4);
				RtpExtHeader *ext = (RtpExtHeader *)(pkt + payloadOffset);
				uint16_t extProfile = ntohl(ext->profile);
				uint16_t extLength = ntohl(ext->length);

				LogAync(
						LOG_DEBUG,
						"RtpSession::RecvRtpPacket( "
						"this : %p, "
						"[%s], "
						"ssrc : 0x%08x(%u), "
						"[Extension], "
						"profile : 0x%04x, "
						"extLength : %u "
						")",
						this,
						PktTypeDesc(ssrc).c_str(),
						ssrc,
						ssrc,
						extProfile,
						extLength
						);
			}

			if ( mpRecvSrtpCtx ) {
				srtp_err_status_t status = srtp_unprotect(mpRecvSrtpCtx, pkt, (int *)&pktSize);
				bFlag = (status == srtp_err_status_ok);

				if( bFlag ) {
					// 模拟丢包
					bFlag = SimPktLost(ssrc, seq);
					if ( bFlag ) {
						// 更新媒体流信息
						UpdateStreamInfo(pkt, pktSize);
					}
				}
			}
		} else {
			LogAync(
					LOG_NOTICE,
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

bool RtpSession::SendRtpPacket(void *pkt, unsigned int& pktSize) {
	bool bFlag = false;
	int sendSize = 0;

	srtp_err_status_t status = srtp_err_status_fail;
	mClientMutex.lock();
	if( mRunning ) {
		status = srtp_protect(mpSendSrtpCtx, pkt, (int *)&pktSize);
	}
	mClientMutex.unlock();

	/**
	 * SendData接口不能放在锁里, 因为外部SendData有可能是同步的实现方式
	 */
    if (status == srtp_err_status_ok) {
    	if( mpRtpSender ) {
    		sendSize = mpRtpSender->SendData((void *)pkt, pktSize);
			if (sendSize == (int)pktSize) {
				bFlag = true;
			}
    	} else {
    		bFlag = false;
    	}
    } else {
    	bFlag = false;
    }

//    if ( !bFlag ) {
//    	RtpHeader *header = (RtpHeader *)pkt;
//
//		LogAync(
//				LOG_DEBUG,
//				"RtpSession::SendRtpPacket( "
//				"this : %p, "
//				"status : %d, "
//				"sendSize : %d, "
//				"pktSize : %d, "
//				"seq : %u, "
//				"timestamp : %u "
//				")",
//				this,
//				status,
//				sendSize,
//				pktSize,
//				ntohs(header->seq),
//				ntohl(header->ts)
//				);
//    }

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

			RtcpPayloadType type = (RtcpPayloadType)((RtcpPacketCommon *)pkt)->header.pt;
			uint32_t ssrc = ntohl(((RtcpPacketCommon *)pkt)->ssrc);

			LogAync(
					LOG_DEBUG,
					"RtpSession::RecvRtcpPacket( "
					"this : %p, "
					"ssrc : 0x%08x(%u), "
					"type : %u "
					")",
					this,
					ssrc,
					ssrc,
					type
					);

			if ( mpRecvSrtpCtx ) {
				srtp_err_status_t status = srtp_unprotect_rtcp(mpRecvSrtpCtx, pkt, (int *)&pktSize);
				bFlag = (status == srtp_err_status_ok);

				if( bFlag ) {
					// 更新媒体流信息
					UpdateStreamInfoWithRtcp(pkt, pktSize);
				}
			}

		} else {
			LogAync(
					LOG_NOTICE,
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

bool RtpSession::SendRtcpPacket(void *pkt, unsigned int& pktSize) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;
	mClientMutex.lock();
	if( mRunning && pktSize > 0 ) {
		status = srtp_protect_rtcp(mpSendSrtpCtx, pkt, (int *)&pktSize);
	}
	mClientMutex.unlock();

	/**
	 * SendData接口不能放在锁里, 因为外部SendData有可能是同步的实现方式
	 */
    if (status == srtp_err_status_ok) {
    	if( mpRtcpSender ) {
    		int sendSize = mpRtcpSender->SendData((void *)pkt, pktSize);
			if (sendSize == (int)pktSize) {
				bFlag = true;
			}
    	} else {
    		bFlag = false;
    	}
    } else {
    	bFlag = false;
    }

	return bFlag;
}

bool RtpSession::SendRtcpPLI(unsigned int mediaSSRC) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;
	char tmp[MTU];
	unsigned int pktSize = 0;

	RtcpPacketPLI pkt = {0};
	pkt.header.version = 2;
	pkt.header.p = 0;
	pkt.header.rc = 1;
	pkt.header.pt = RtcpPayloadTypePSFB;
	pkt.header.length = 2; // (1 * ssrc(32bit)) + (1 * media_ssrc(32bit))
	pkt.header.length = htons(pkt.header.length);
	pkt.ssrc = htonl(RTCP_SSRC);
	pkt.media_ssrc = htonl(mediaSSRC);

	memcpy(tmp, (void *)&pkt, sizeof(RtcpPacketPLI));
	pktSize += sizeof(RtcpPacketPLI);

	bFlag = SendRtcpPacket((void *)tmp, pktSize);

	return bFlag;
}

bool RtpSession::SendRtcpFIR(unsigned int mediaSSRC) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;
	char tmp[MTU];
	unsigned pktSize = 0;

	RtcpPacketPLI pkt = {0};
	pkt.header.version = 2;
	pkt.header.p = 0;
	pkt.header.rc = 4;
	pkt.header.pt = RtcpPayloadTypePSFB;
	pkt.header.length = 4; // (1 * ssrc(32bit)) + (1 * media_ssrc(32bit) + 1 * (1 * FIR(2 * 32bit)))
	pkt.header.length = htons(pkt.header.length);
	pkt.ssrc = htonl(RTCP_SSRC);
	pkt.media_ssrc = htonl(mediaSSRC);

	RtcpPacketFIRItem item = {0};
	item.media_ssrc = htonl(mediaSSRC);
	item.seq = ++mFirSeq;

	memcpy(tmp, (void *)&pkt, sizeof(RtcpPacketPLI));
	pktSize += sizeof(RtcpPacketPLI);
	memcpy(tmp + pktSize, (void *)&item, sizeof(RtcpPacketFIRItem));
	pktSize += sizeof(RtcpPacketFIRItem);

	bFlag = SendRtcpPacket((void *)tmp, pktSize);

	return bFlag;
}

bool RtpSession::SendRtcpNack(unsigned int mediaSSRC, void* nacks, int size) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;
	char tmp[MTU];
	unsigned int pktSize = 0;

	RtcpPacketPLI pkt = {0};
	pkt.header.version = 2;
	pkt.header.p = 0;
	pkt.header.rc = 1;
	pkt.header.pt = RtcpPayloadTypeRTPFB;
	pkt.header.length = 2 + size; // (1 * ssrc(32bit)) + (1 * media_ssrc(32bit) + (size * NACK(32bit)))
	pkt.header.length = htons(pkt.header.length);
	pkt.ssrc = htonl(RTCP_SSRC);
	pkt.media_ssrc = htonl(mediaSSRC);

	memcpy(tmp, (void *)&pkt, sizeof(RtcpPacketPLI));
	pktSize += sizeof(RtcpPacketPLI);

	RtcpPacketNackItem* items = (RtcpPacketNackItem *)nacks;
	for (int i = 0; i < size; i++) {
		RtcpPacketNackItem *item = (items + i);

//			LogAync(
//					LOG_WARNING,
//					"RtpSession::SendRtcpNack( "
//					"this : %p, "
//					"mediaSSRC : 0x%x(%u), "
//					"seq : %d, "
//					"bitmap : 0x%x "
//					")",
//					this,
//					mediaSSRC,
//					mediaSSRC,
//					item->seq,
//					item->bitmap
//					);

		item->seq = htons(item->seq);
		item->bitmap = htons(item->bitmap);

		memcpy(tmp + pktSize, (void *)item, sizeof(RtcpPacketNackItem));
		pktSize += sizeof(RtcpPacketNackItem);
	}

	bFlag = SendRtcpPacket((void *)tmp, pktSize);

	return bFlag;
}

bool RtpSession::SendRtcpRR() {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;
	char tmp[MTU];
	unsigned int pktSize = 0;

	RtcpPacketCommon pkt = {0};
	pkt.header.version = 2;
	pkt.header.p = 0;
	pkt.header.rc = 1;
	pkt.header.pt = RtcpPayloadTypeRR;
	pkt.header.length = 1 + 2 * 6; // (1 * ssrc(32bit)) + (2 * RR(6 * 32bit))
	pkt.header.length = htons(pkt.header.length);
	pkt.ssrc = htonl(RTCP_SSRC);
	memcpy(tmp, (void *)&pkt, sizeof(RtcpPacketCommon));
	pktSize += sizeof(RtcpPacketCommon);

	// Audio RR
	RtcpPacketRR rr_audio = {0};
	rr_audio.media_ssrc = htonl(mAudioSSRC);
	/**
	 * 丢包率: RTP包丢失个数 / 期望接收的RTP包个数
	 * RTP包丢失个数 = 期望接收的RTP包个数 - 实际收到的RTP包个数
	 * 期望接收的RTP包个数 = 当前最大sequence - 上次最大sequence
	 * 实际收到的RTP包个数 = 正常有序RTP包 + 重传包
	 */
	unsigned int expected = mAudioMaxSeq - mAudioMaxSeqLast;
	unsigned int actual = mAudioTotalRecvPacket - mAudioTotalRecvPacketLast;
	mAudioTotalRecvPacketLast = mAudioTotalRecvPacket;
	rr_audio.fraction = 256 * 1.0f * (expected - actual) / expected;
	rr_audio.packet_lost = actual;
	rr_audio.max_seq = htons(mAudioMaxSeq);
	/**
	 * 抖动: 一对数据包在接收端与发送端的数据包时间间距之差
	 * R: RTP包接收的时间戳
	 * S: RTP包发送的时间戳
	 * 抖动(i, j) = D(i, j) = |(Rj - Ri) - (Sj - Si)| = |(Rj - Sj) - (Ri - Si)|
	 * 抖动函数 J(i) = J(i - 1) + (|D(i - 1, i)| - J(i - 1)) / 16
	 *
	 * 暂时先不计算, 需要用到RTP扩展头 jitter_q4_transmission_time_offset_
	 */
	rr_audio.jitter = 0;

	rr_audio.lsr = mAudioLSR;
	rr_audio.dlsr = mAudioDLSR;
	memcpy(tmp + pktSize, (void *)&rr_audio, sizeof(RtcpPacketRR));
	pktSize += sizeof(RtcpPacketRR);

	// Video RR
	RtcpPacketRR rr_video = {0};
	rr_video.media_ssrc = htonl(mVideoSSRC);
	expected = mVideoMaxSeq - mVideoMaxSeqLast;
	actual = mVideoTotalRecvPacket - mVideoTotalRecvPacketLast;
	mVideoTotalRecvPacketLast = mVideoTotalRecvPacket;
	rr_video.fraction = 256 * 1.0f * (expected - actual) / expected;
	rr_video.packet_lost = actual;
	rr_video.max_seq = htons(mVideoMaxSeq);
	rr_video.jitter = 0;
	rr_video.lsr = mVideoLSR;
	rr_video.dlsr = mVideoDLSR;
	memcpy(tmp + pktSize, (void *)&rr_video, sizeof(RtcpPacketRR));
	pktSize += sizeof(RtcpPacketRR);

	bFlag = SendRtcpPacket((void *)tmp, pktSize);

	return bFlag;
}

void RtpSession::UpdateStreamInfo(const void *pkt, unsigned int pktSize) {
	unsigned short seq = ntohs(((RtpPacket *)pkt)->header.seq);
	uint32_t ts = ntohl(((RtpPacket *)pkt)->header.ts);
	uint32_t ssrc = ntohl(((RtpPacket *)pkt)->header.ssrc);

	if ( IsAudioPkt(ssrc) ) {
		if ( mAudioMaxSeq == 0 ) {
			mAudioMaxSeq = seq;
		}
		if ( mAudioMaxTimestamp == 0 ) {
			mAudioMaxTimestamp = ts;
		}

		LogAync(
				LOG_DEBUG,
				"RtpSession::UpdateStreamInfo( "
				"this : %p, "
				"[Audio], "
				"ssrc : 0x%08x(%u), "
				"seq : %u, "
				"timestamp : %u, "
				"mAudioMaxSeq : %u, "
				"mAudioMaxTimestamp : %u, "
				"mAudioTotalRecvPacket : %u "
				")",
				this,
				ssrc,
				ssrc,
				seq,
				ts,
				mAudioMaxSeq,
				mAudioMaxTimestamp,
				mAudioTotalRecvPacket
				);

		// 更新丢包信息
		UpdateLossPacket(ssrc, seq, mAudioMaxSeq, ts, mAudioMaxTimestamp);
		// 更新音频时间戳和帧号
		mAudioMaxSeq = MAX(mAudioMaxSeq, seq);
		mAudioMaxTimestamp = MAX(mAudioMaxTimestamp, seq);
		// 更新收到的数据包
		mAudioTotalRecvPacket++;

	} else {
		if ( mVideoMaxSeq == 0 ) {
			mVideoMaxSeq = seq;
		}
		if ( mVideoMaxTimestamp == 0 ) {
			mVideoMaxTimestamp = ts;
		}

		LogAync(
				LOG_DEBUG,
				"RtpSession::UpdateStreamInfo( "
				"this : %p, "
				"[Video], "
				"ssrc : 0x%08x(%u), "
				"seq : %u, "
				"timestamp : %u, "
				"mVideoMaxSeq : %u, "
				"mVideoMaxTimestamp : %u, "
				"mVideoTotalRecvPacket : %u "
				")",
				this,
				ssrc,
				ssrc,
				seq,
				ts,
				mVideoMaxSeq,
				mVideoMaxTimestamp,
				mVideoTotalRecvPacket
				);

		/**
		 * 临时方案刷新关键帧, 正常应该根据[丢包/延迟/解码情况]判断是否请求关键帧
		 */
		if( ((RtpPacket *)pkt)->header.m ) {
			++mVideoFrameCount;
			if( mVideoFrameCount % 30 == 0 ) {
				mVideoFrameCount = 0;
				// 强制刷新一次视频信息(包含视频[宽高/关键帧])
//				SendRtcpFIR(ssrc);
				// 强制刷新一次关键帧
				SendRtcpPLI(ssrc);
//				// 模拟一次丢包
//				SimPktLost();
			}
		}

		// 更新丢包信息
		UpdateLossPacket(ssrc, seq, mVideoMaxSeq, ts, mVideoMaxTimestamp);
		// 更新视频时间戳和帧号
		mVideoMaxSeq = MAX(mVideoMaxSeq, seq);
		mVideoMaxTimestamp = MAX(mVideoMaxTimestamp, ts);
		// 更新收到的数据包
		mVideoTotalRecvPacket++;
	}
}

bool RtpSession::UpdateLossPacket(unsigned int ssrc, unsigned int seq, unsigned int lastMaxSeq, unsigned int ts, unsigned int lastMaxTs) {
	/**
	 * 暂时只做简单处理, 直接发送Nack
	 *
	 * 正常算法
	 * 	1.判断丢包队列数量是否过大, 过大则清空丢包队列, 不进行Nack, 直接请求关键帧(PLI)
	 * 	2.判断丢包队列最大时长(根据RTT计算)是否过大, 过大则清空丢包队列, 不进行Nack, 直接请求关键帧(PLI)
	 *	3.从(seq) - ((last seq) + 1)的序号开始到seq - 1, 放进Nack队列, 等待重传, 等待时间(根据RTT计算)
	 *	4.在独立的处理线程处理丢包队, 发送Nack
	 */
	bool bFlag = false;

	// 开始产生重传队列
	int rtpLostSize = (int)(seq - (lastMaxSeq + 1));
	if ( rtpLostSize > 0 ) {
		bFlag = true;

		if ( rtpLostSize < 300 ) {
			// 每个int可以支持最多17个seq
			int quotient = rtpLostSize / 17;
			int remainder = (rtpLostSize % 17);
			int nackItemTotal = (remainder == 0)?quotient:(quotient + 1);

			// 开始帧号
			int rtpStartSeq = lastMaxSeq + 1;
			// 剩余长度
			int rtpLastCount = rtpLostSize;

			LogAync(
					LOG_INFO,
					"RtpSession::UpdateLossPacket( "
					"this : %p, "
					"[%s], "
					"[Packet Loss], "
					"ssrc : 0x%08x(%u), "
					"rtpLostSize : %d, "
					"nackItemTotal : %d "
					")",
					this,
					PktTypeDesc(ssrc).c_str(),
					ssrc,
					ssrc,
					rtpLostSize,
					nackItemTotal
					);

			RtcpPacketNackItem *items = new RtcpPacketNackItem[nackItemTotal];
			for (int i = 0; i < nackItemTotal; i++) {
				// 开始序号
				items[i].seq = rtpStartSeq;
				// 有多少个连续的丢包
				int rtpPktLostLen = 0;
				if ( rtpLastCount >= 17 ) {
					rtpPktLostLen = 16;
					items[i].bitmap = 0xFFFF;
				} else {
					rtpPktLostLen = rtpLastCount;
					items[i].bitmap = 0xFFFF >> (16 - rtpPktLostLen);
				}

				LogAync(
						LOG_INFO,
						"RtpSession::UpdateLossPacket( "
						"this : %p, "
						"[%s], "
						"[Packet Loss], "
						"ssrc : 0x%08x(%u), "
						"rtpStartSeq : %d, "
						"rtpLastCount : %d, "
						"rtpPktLostLen : %d, "
						"bitmap : 0x%x "
						")",
						this,
						PktTypeDesc(ssrc).c_str(),
						ssrc,
						ssrc,
						rtpStartSeq,
						rtpLastCount,
						rtpPktLostLen,
						items[i].bitmap
						);

				rtpStartSeq += 17;
				rtpLastCount -= 17;
			}

			SendRtcpNack(ssrc, (void *)items, nackItemTotal);
			delete[] items;
		} else {
			// 丢包太多, 强刷关键帧
			LogAync(
					LOG_NOTICE,
					"RtpSession::UpdateLossPacket( "
					"this : %p, "
					"[%s], "
					"[Packet Loss Too Many], "
					"ssrc : 0x%08x(%u), "
					"rtpLostSize : %u "
					")",
					this,
					PktTypeDesc(ssrc).c_str(),
					ssrc,
					ssrc,
					rtpLostSize
					);

			SendRtcpPLI(ssrc);
		}
	}

	return bFlag;
}

void RtpSession::UpdateStreamInfoWithRtcp(const void *pkt, unsigned int pktSize) {
	RtcpPayloadType type = (RtcpPayloadType)((RtcpPacketCommon *)pkt)->header.pt;
	uint32_t ssrc = ntohl(((RtcpPacketCommon *)pkt)->ssrc);

	switch (type) {
	case RtcpPayloadTypeSR:{
		RtcpPacketSR *sr = (RtcpPacketSR *)pkt;
		uint32_t ntp_ts_most = ntohl(sr->ntp_ts_most);
		uint32_t ntp_ts_least = ntohl(sr->ntp_ts_least);
		uint32_t rtp_ts = ntohl(sr->rtp_ts);
		uint32_t sender_packet_count = ntohl(sr->sender_packet_count);
		uint32_t sender_octet_count = ntohl(sr->sender_octet_count);

		NtpTime lsr = NtpTime(ntp_ts_most, ntp_ts_least);
		NtpTime now = RealTimeClock::CurrentNtpTime();

		uint32_t seconds = now.seconds() - lsr.seconds();
		uint32_t fractions = 0;
		if ( now.fractions() < lsr.fractions() ) {
			fractions = lsr.fractions() - now.fractions();
			seconds--;
		} else {
			fractions = now.fractions() - lsr.fractions();
		}

		NtpTime dlsr = NtpTime(seconds, fractions);
		// 计算RR中的LSR
		uint32_t rr_lsr = ((lsr.seconds() & 0x0000FFFF) << 16) + ((lsr.fractions() & 0xFFFF0000) >> 16);
		// 计算RR中的DLSR
		uint32_t rr_dlsr = ((dlsr.seconds() & 0x0000FFFF) << 16) + ((dlsr.fractions() & 0xFFFF0000) >> 16);
		uint64_t rr_dlsr_ms = dlsr.ToMs();

		if ( IsAudioPkt(ssrc) ) {
			mAudioLSR = rr_lsr;
			mAudioLSR = rr_dlsr;
		} else {
			mVideoLSR = rr_lsr;
			mVideoDLSR = rr_dlsr;
		}

		LogAync(
				LOG_INFO,
				"RtpSession::UpdateStreamInfoWithRtcp( "
				"this : %p, "
				"[%s], "
				"[Recv SR], "
				"ssrc : 0x%08x(%u), "
				"ntp_ts_most : 0x%08x, "
				"ntp_ts_least : 0x%08x, "
				"rtp_ts : %u, "
				"sender_packet_count : %u, "
				"sender_octet_count : %u, "
				"rr_lsr : 0x%08x, "
				"rr_dlsr : 0x%08x(%llums) "
				")",
				this,
				PktTypeDesc(ssrc).c_str(),
				ssrc,
				ssrc,
				ntp_ts_most,
				ntp_ts_least,
				rtp_ts,
				sender_packet_count,
				sender_octet_count,
				rr_lsr,
				rr_dlsr,
				rr_dlsr_ms
				);

	}break;
	default:break;
	}
}

string RtpSession::PktTypeDesc(unsigned int ssrc) {
	return IsAudioPkt(ssrc)?"Audio":"Video";
}

bool RtpSession::IsAudioPkt(unsigned int ssrc) {
	return (ssrc == mAudioSSRC);
}

bool RtpSession::SimPktLost(unsigned int ssrc, unsigned int seq) {
	bool bFlag = true;
	if ( mbSimLost ) {
		if ( IsAudioPkt(ssrc) ) {
			if ( mbAudioAbandonning ) {
				if ( ++mAudioAbandonCount <= mAudioAbandonTotal ) {
					bFlag = false;
				} else {
					mbAudioAbandonning = false;
					mAudioAbandonTotal = 0;
					mAudioAbandonCount = 0;
				}
			} else {
				int check = rand() % mAudioLostSeed;
				if ( check == 0 ) {
					// 模拟丢包
					mbAudioAbandonning = true;
					mAudioAbandonTotal = rand() % mAudioLostSize;
					mAudioAbandonCount = 0;
					bFlag = false;

					LogAync(
							LOG_NOTICE,
							"RtpSession::SimPktLost( "
							"this : %p, "
							"[%s], "
							"ssrc : 0x%08x(%u), "
							"seq : %u, "
							"total : %u "
							")",
							this,
							PktTypeDesc(ssrc).c_str(),
							ssrc,
							ssrc,
							seq,
							mAudioAbandonTotal
							);
				}
			}
		} else {
			if ( mbVideoAbandonning ) {
				if ( ++mVideoAbandonCount <= mVideoAbandonTotal ) {
					bFlag = false;
				} else {
					mbVideoAbandonning = false;
					mVideoAbandonTotal = 0;
					mVideoAbandonCount = 0;
				}
			} else {
				int check = rand() % mVideoLostSeed;
				if ( check == 0 ) {
					// 模拟丢包
					mbVideoAbandonning = true;
					mVideoAbandonTotal = rand() % mVideoLostSize;
					mVideoAbandonCount = 0;
					bFlag = false;

					LogAync(
							LOG_NOTICE,
							"RtpSession::SimPktLost( "
							"this : %p, "
							"[%s], "
							"ssrc : 0x%08x(%u), "
							"seq : %u, "
							"total : %u "
							")",
							this,
							PktTypeDesc(ssrc).c_str(),
							ssrc,
							ssrc,
							seq,
							mVideoAbandonTotal
							);
				}
			}
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
	if( len > sizeof(RtpHeader) ) {
		RtpHeader *header = (RtpHeader *)frame;
		bFlag = ((header->pt < 64) || ((header->pt >= 96) && (header->pt < 200)));
	}
	return bFlag;
}

bool RtpSession::IsRtcp(const char *frame, unsigned len) {
	bool bFlag = false;
	if( len > sizeof(RtcpHeader) ) {
		RtcpHeader *header = (RtcpHeader *)frame;
		bFlag = ((header->pt >= 200) && (header->pt <= 210));
	}
	return bFlag;
}

} /* namespace mediaserver */
