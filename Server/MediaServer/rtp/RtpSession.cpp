/*
 * RtpSession.cpp
 *
 *  Created on: 2019/06/20
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "RtpSession.h"

#include <rtp/packet/RtpPacket.h>
#include <rtp/packet/rtp_packet_received.h>
#include <rtp/packet/Pli.h>
#include <rtp/packet/Fir.h>
#include <rtp/packet/Nack.h>
#include <rtp/packet/Remb.h>
#include <rtp/packet/CommonHeader.h>
#include <rtp/packet/ExtendedReports.h>
#include <rtp/packet/SenderReport.h>
#include <rtp/packet/receiver_report.h>
#include <rtp/packet/sdes.h>

#include <rtp/base/clock.h>
#include <rtp/include/time_util.h>

#include <include/CommonHeader.h>

#include <common/LogManager.h>
#include <common/Math.h>
#include <common/CommonFunc.h>
#include <common/Arithmetic.h>
#include <common/Math.h>

// libsrtp
#include <srtp.h>
#include <srtp_priv.h>

#include <string>
using namespace std;
/*
 * SRTP key size
 */
#define MAX_KEY_LEN 96
static char kEmptyKey[MAX_KEY_LEN] = { 0 };

namespace mediaserver {

#define RTCP_HEADER_LENGTH 4
#define RTCP_COMMON_FEEDBACK_LENGTH 8

#define RTCP_SSRC 0x12345678

#pragma pack(push, 1)
typedef struct {
	unsigned char cc :4; /* CSRC count             */
	unsigned char x :1; /* header extension flag  */
	unsigned char p :1; /* padding flag           */
	unsigned char version :2; /* protocol version       */
	unsigned char pt :7; /* payload type           */
	unsigned char m :1; /* marker bit             */
	uint16_t seq; /* sequence number        */
	uint32_t ts; /* timestamp              */
	uint32_t ssrc; /* synchronization source */
} RtpHeader; /* BIG END */

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
	unsigned char rc :5; /* fmt 					 				 */
	unsigned char p :1; /* padding flag           				 */
	unsigned char version :2; /* protocol version       				 */
	unsigned char pt :8; /* payload					 			 */
	uint16_t length; /* count of media ssrc, each one is 32bit */
} RtcpHeader; /* BIG END */

typedef struct {
	uint32_t media_ssrc;
	unsigned char fraction; /* fraction lost	 							*/
	uint32_t packet_lost :24; /* cumulative packet lost 						*/
	uint32_t max_seq; /* extended highest sequence number received	*/
	uint32_t jitter; /* jitter		 								*/
	uint32_t lsr; /* last SR		 								*/
	uint32_t dlsr; /* delay last SR		 						*/
} RtcpPacketRR; /* BIG END */

#pragma pack(pop)

bool RtpSession::GobalInit() {
	bool bFlag = false;

	/* initialize srtp library */
	srtp_err_status_t status = srtp_init();
	bFlag = (status == srtp_err_status_ok);

	if (!bFlag) {
		LogAync(LOG_ALERT, "RtpSession::GobalInit( "
				"[%s] "
				")", FLAG_2_STRING(bFlag));
	}

	return bFlag;
}

RtpSession::RtpSession() :
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
	if (mpRecvPolicy) {
		delete mpRecvPolicy;
		mpRecvPolicy = NULL;
	}

	if (mpSendPolicy) {
		delete mpSendPolicy;
		mpSendPolicy = NULL;
	}
}

void RtpSession::Reset() {
	// Video
	mVideoMaxTimestamp = 0;
	mVideoPLITimestamp = 0;
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

	mSendRtcpXr = false;

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

void RtpSession::RegisterVideoExtensions(
		const vector<RtpExtension>& extensions) {
	for (vector<RtpExtension>::const_iterator itr = extensions.cbegin();
			itr != extensions.cend(); itr++) {
		LogAync(LOG_INFO, "RtpSession::RegisterVideoExtensions( "
				"this : %p, "
				"id : %d, "
				"uri : %s "
				")", this, itr->id, itr->uri.c_str());
		mVideoExtensionMap.RegisterByUri(itr->id, itr->uri);
	}
}

void RtpSession::RegisterAudioExtensions(
		const vector<RtpExtension>& extensions) {
	for (vector<RtpExtension>::const_iterator itr = extensions.cbegin();
			itr != extensions.cend(); itr++) {
		LogAync(LOG_INFO, "RtpSession::RegisterAudioExtensions( "
				"this : %p, "
				"id : %d, "
				"uri : %s "
				")", this, itr->id, itr->uri.c_str());
//		mAudioExtensionMap.RegisterByUri(itr->id, itr->uri);
	}
}

bool RtpSession::Start(char *localKey, int localSize, char *remoteKey,
		int remoteSize) {
	bool bFlag = false;

	Arithmetic art;
	LogAync(LOG_INFO, "RtpSession::Start( "
			"this : %p, "
			"localKey : %s, "
			"remoteKey : %s "
			")", this, art.AsciiToHexWithSep(localKey, localSize).c_str(),
			art.AsciiToHexWithSep(remoteKey, remoteSize).c_str());

	srtp_err_status_t status = srtp_err_status_ok;

	mClientMutex.lock();
	if (mRunning) {
		Stop();
	}

	mRunning = true;

	bFlag = StartSend(localKey, localSize);
	bFlag &= StartRecv(remoteKey, remoteSize);

	if (bFlag) {
		LogAync(LOG_INFO, "RtpSession::Start( "
				"this : %p, "
				"[OK] "
				")", this);
	} else {
		LogAync(LOG_ALERT, "RtpSession::Start( "
				"this : %p, "
				"[Fail], "
				"status : %d "
				")", this, status);
		Stop();
	}

	mClientMutex.unlock();

	return bFlag;
}

void RtpSession::Stop() {
	LogAync(LOG_INFO, "RtpSession::Stop( "
			"this : %p "
			")", this);

	mClientMutex.lock();
	if (mRunning) {
		mRunning = false;

		StopRecv();
		StopSend();

		mVideoExtensionMap.Clear();
		mAudioExtensionMap.Clear();
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

	LogAync(LOG_INFO, "RtpSession::Stop( "
			"this : %p "
			"[OK] "
			")", this);
}

void RtpSession::SetSimLostParam(bool bSimLost, unsigned int audioLostSeed,
		unsigned int audioLostSize, unsigned int videoLostSeed,
		unsigned int videoLostSize) {
	mbSimLost = bSimLost;
	mAudioLostSeed = audioLostSeed;
	mAudioLostSize = audioLostSize;
	mVideoLostSeed = videoLostSeed;
	mVideoLostSize = videoLostSize;
}

bool RtpSession::StartSend(char *localKey, int size) {
	bool bFlag = true;

	Arithmetic art;
	LogAync(LOG_DEBUG, "RtpSession::StartSend( "
			"this : %p, "
			"size : %d, "
			"localKey : %s "
			")", this, size, art.AsciiToHexWithSep(localKey, size).c_str());

	// Initialize libsrtp
	memset(mpSendPolicy, 0x0, sizeof(srtp_policy_t));

	if (localKey && size > 0) {
		srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&mpSendPolicy->rtp);
		srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&mpSendPolicy->rtcp);
		mpSendPolicy->key = (uint8_t *) localKey;
	} else {
		srtp_crypto_policy_set_null_cipher_hmac_null(&mpSendPolicy->rtp);
		srtp_crypto_policy_set_null_cipher_hmac_null(&mpSendPolicy->rtcp);
		mpSendPolicy->key = (uint8_t *) kEmptyKey;
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

	if (bFlag) {
		LogAync(LOG_DEBUG, "RtpSession::StartSend( "
				"this : %p, "
				"[OK] "
				")", this);
	} else {
		LogAync(LOG_ALERT, "RtpSession::StartSend( "
				"this : %p, "
				"[Fail], "
				"status : %d "
				")", this, status);
	}

	return bFlag;
}

void RtpSession::StopSend() {
	if (mpSendSrtpCtx) {
		srtp_dealloc(mpSendSrtpCtx);
		mpSendSrtpCtx = NULL;
	}

	LogAync(LOG_DEBUG, "RtpSession::StopSend( "
			"this : %p, "
			"[OK] "
			")", this);
}

bool RtpSession::StartRecv(char *remoteKey, int size) {
	bool bFlag = true;

	Arithmetic art;
	LogAync(LOG_DEBUG, "RtpSession::StartRecv( "
			"this : %p, "
			"size : %d, "
			"remoteKey : %s "
			")", this, size, art.AsciiToHexWithSep(remoteKey, size).c_str());

	// Initialize libsrtp
	memset(mpRecvPolicy, 0x0, sizeof(srtp_policy_t));

	if (remoteKey && size > 0) {
		srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&mpRecvPolicy->rtp);
		srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&mpRecvPolicy->rtcp);
		mpRecvPolicy->key = (uint8_t *) remoteKey;

	} else {
		srtp_crypto_policy_set_null_cipher_hmac_null(&mpRecvPolicy->rtp);
		srtp_crypto_policy_set_null_cipher_hmac_null(&mpRecvPolicy->rtcp);
		mpRecvPolicy->key = (uint8_t *) kEmptyKey;
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

	if (bFlag) {
		LogAync(LOG_DEBUG, "RtpSession::StartRecv( "
				"this : %p, "
				"[OK] "
				")", this);
	} else {
		LogAync(LOG_ALERT, "RtpSession::StartRecv( "
				"this : %p, "
				"[Fail], "
				"status : %d "
				")", this, status);
	}

	return bFlag;
}

void RtpSession::StopRecv() {
	if (mpRecvSrtpCtx) {
		srtp_dealloc(mpRecvSrtpCtx);
		mpRecvSrtpCtx = NULL;
	}

	LogAync(LOG_DEBUG, "RtpSession::StopRecv( "
			"this : %p, "
			"[OK] "
			")", this);
}

bool RtpSession::RecvRtpPacket(const char* frame, unsigned int size, void *pkt,
		unsigned int& pktSize) {
	bool bFlag = false;

	mClientMutex.lock();
	if (mRunning) {
		srtp_err_status_t status = srtp_err_status_ok;
		if (IsRtp(frame, size)) {
			memcpy((void *) pkt, (void *) frame, size);
			pktSize = size;

			int64_t recvTime = getCurrentTime();
			if (mpRecvSrtpCtx) {
				srtp_err_status_t status = srtp_unprotect(mpRecvSrtpCtx, pkt,
						(int *) &pktSize);
				bFlag = (status == srtp_err_status_ok);

				if (bFlag) {
//					RtpPacket rtpPkt;
					RtpPacketReceived rtpPkt(&mVideoExtensionMap);
					bool bFlag = rtpPkt.Parse((const uint8_t *) pkt, pktSize);
					if (bFlag) {
//						LogAync(LOG_DEBUG, "RtpSession::RecvRtpPacket( "
//								"this : %p, "
//								"[%s], "
//								"media_ssrc : 0x%08x(%u), "
//								"x : %u, "
//								"seq : %u, "
//								"ts : %u, "
//								"recvTime : %lld, "
//								"pktSize : %d"
//								"%s"
//								")", this, PktTypeDesc(rtpPkt.ssrc_).c_str(), rtpPkt.ssrc_,
//								rtpPkt.ssrc_, rtpPkt.has_extension_,
//								rtpPkt.sequence_number_, rtpPkt.timestamp_,
//								recvTime, pktSize,
//								rtpPkt.marker_ ? ", [Mark] " : " ");
						RTPHeader header;
						rtpPkt.GetHeader(&header);
						LogAync(LOG_DEBUG, "RtpSession::RecvRtpPacket( "
								"this : %p, "
								"[%s], "
								"media_ssrc : 0x%08x(%u), "
								"x : %u, "
								"seq : %u, "
								"ts : %u, "
								"abs : %lld, "
								"recvTime : %lld, "
								"pktSize : %d"
								"%s"
								")", this, PktTypeDesc(rtpPkt.ssrc_).c_str(),
								rtpPkt.ssrc_, rtpPkt.ssrc_,
								rtpPkt.has_extension_, header.sequenceNumber,
								header.timestamp,
								header.extension.GetAbsoluteSendTimestamp().ms(),
								recvTime, pktSize,
								header.markerBit ? ", [Mark] " : " ");
						// 模拟丢包
						bFlag = SimPktLost(rtpPkt.ssrc_,
								rtpPkt.sequence_number_);
						if (bFlag) {
							// 更新媒体流信息
							UpdateStreamInfo(&rtpPkt, recvTime);
						}
					}
				}
			}
		} else {
			LogAync(LOG_NOTICE, "RtpSession::RecvRtpPacket( "
					"this : %p, "
					"[Ignore frame before Handshake] "
					")", this);
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
	if (mRunning) {
		status = srtp_protect(mpSendSrtpCtx, pkt, (int *) &pktSize);
	}
	mClientMutex.unlock();

	/**
	 * SendData接口不能放在锁里, 因为外部SendData有可能是同步的实现方式
	 */
	if (status == srtp_err_status_ok) {
		if (mpRtpSender) {
			sendSize = mpRtpSender->SendData((void *) pkt, pktSize);
			if (sendSize == (int) pktSize) {
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

bool RtpSession::RecvRtcpPacket(const char* frame, unsigned int size, void *pkt,
		unsigned int& pktSize) {
	bool bFlag = false;

	mClientMutex.lock();
	if (mRunning) {
		srtp_err_status_t status = srtp_err_status_ok;
		if (IsRtcp(frame, size)) {
			memcpy((void *) pkt, (void *) frame, size);
			pktSize = size;

//			RtcpPayloadType type = (RtcpPayloadType)((RtcpHeader *)pkt)->pt;
//
//			LogAync(
//					LOG_DEBUG,
//					"RtpSession::RecvRtcpPacket( "
//					"this : %p, "
//					"type : %u "
//					")",
//					this,
//					type
//					);

			if (mpRecvSrtpCtx) {
				srtp_err_status_t status = srtp_unprotect_rtcp(mpRecvSrtpCtx,
						pkt, (int *) &pktSize);
				bFlag = (status == srtp_err_status_ok);

				if (bFlag) {
					// 更新媒体流信息
					UpdateStreamInfoWithRtcp(pkt, pktSize);
				}
			}

		} else {
			LogAync(LOG_NOTICE, "RtpSession::RecvRtcpPacket( "
					"this : %p, "
					"[Ignore frame before Handshake] "
					")", this);
		}
	}
	mClientMutex.unlock();

	return bFlag;
}

bool RtpSession::SendRtcpPacket(void *pkt, unsigned int& pktSize) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;
	mClientMutex.lock();
	if (mRunning && pktSize > 0) {
		status = srtp_protect_rtcp(mpSendSrtpCtx, pkt, (int *) &pktSize);
	}
	mClientMutex.unlock();

	/**
	 * SendData接口不能放在锁里, 因为外部SendData有可能是同步的实现方式
	 */
	if (status == srtp_err_status_ok) {
		if (mpRtcpSender) {
			int sendSize = mpRtcpSender->SendData((void *) pkt, pktSize);
			if (sendSize == (int) pktSize) {
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

bool RtpSession::SendRtcpPLI(unsigned int media_ssrc) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;

	uint8_t buffer[MTU];
	size_t bufferSize = 0;

	Pli pli;
	pli.sender_ssrc_ = RTCP_SSRC;
	pli.media_ssrc_ = media_ssrc;
	bFlag = pli.Create((uint8_t *) buffer, &bufferSize, MTU);

	LogAync(LOG_DEBUG, "RtpSession::SendRtcpPLI( "
			"this : %p, "
			"[%s], "
			"media_ssrc : 0x%08x(%u), "
			"bufferSize : %u "
			")", this, PktTypeDesc(media_ssrc).c_str(), media_ssrc, media_ssrc, bufferSize);

	if (bFlag) {
		unsigned int pktSize = (unsigned int) bufferSize;
		bFlag = SendRtcpPacket((void *) buffer, pktSize);
	}

	return bFlag;
}

bool RtpSession::SendRtcpFIR(unsigned int media_ssrc) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;

	uint8_t buffer[MTU];
	size_t bufferSize = 0;

	Fir fir;
	fir.sender_ssrc_ = RTCP_SSRC;
	fir.AddRequestTo(media_ssrc, ++mFirSeq);
	bFlag = fir.Create((uint8_t *) buffer, &bufferSize, MTU);

	LogAync(LOG_DEBUG, "RtpSession::SendRtcpFIR( "
			"this : %p, "
			"[%s], "
			"media_ssrc : 0x%08x(%u), "
			"bufferSize : %u "
			")", this, PktTypeDesc(media_ssrc).c_str(), media_ssrc, media_ssrc, bufferSize);

	if (bFlag) {
		unsigned int pktSize = (unsigned int) bufferSize;
		bFlag = SendRtcpPacket((void *) buffer, pktSize);
	}

	return bFlag;
}

bool RtpSession::SendRtcpNack(unsigned int media_ssrc, unsigned int start,
		unsigned int size) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;

	uint8_t buffer[MTU];
	size_t bufferSize = 0;

	Nack nack;
	nack.sender_ssrc_ = RTCP_SSRC;
	nack.media_ssrc_ = media_ssrc;
	nack.SetPacketIdsWithStart(start, size);
	bFlag = nack.Create(buffer, &bufferSize, MTU);

	LogAync(LOG_DEBUG, "RtpSession::SendRtcpNack( "
			"this : %p, "
			"[%s], "
			"ssrc : 0x%08x(%u) "
			"bufferSize : %u, "
			"start : %u, "
			"size : %u "
			")", this, PktTypeDesc(media_ssrc).c_str(), media_ssrc, media_ssrc, bufferSize,
			start, size);

	if (bFlag) {
		unsigned int pktSize = (unsigned int) bufferSize;
		bFlag = SendRtcpPacket((void *) buffer, pktSize);
	}

	return bFlag;
}

bool RtpSession::SendRtcpRemb(unsigned int media_ssrc,
		unsigned long long bitrate) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;

	uint8_t buffer[MTU];
	size_t bufferSize = 0;

	Remb remb;
	remb.sender_ssrc_ = RTCP_SSRC;
	remb.SetSsrcs(std::vector<uint32_t>(1, media_ssrc));
	remb.bitrate_bps_ = bitrate;
	bFlag = remb.Create((uint8_t *) buffer, &bufferSize, MTU);

	LogAync(LOG_DEBUG, "RtpSession::SendRtcpRemb( "
			"this : %p, "
			"[%s], "
			"media_ssrc : 0x%08x(%u), "
			"bufferSize : %u, "
			"bitrate : %llu "
			")", this, PktTypeDesc(media_ssrc).c_str(), media_ssrc, media_ssrc, bufferSize,
			bitrate);

	if (bFlag) {
		unsigned int pktSize = (unsigned int) bufferSize;
		bFlag = SendRtcpPacket((void *) buffer, pktSize);
	}

	return bFlag;
}

bool RtpSession::SendRtcpTccFB(unsigned int media_ssrc) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;

	return bFlag;
}

bool RtpSession::SendRtcpXr() {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;

	uint8_t buffer[MTU];
	size_t bufferSize = 0;

	ExtendedReports xr;
	xr.SetSenderSsrc(RTCP_SSRC);

//	if (!mSendRtcpXr) {
		Rrtr rrtr;
		Clock *clock = Clock::GetRealTimeClock();
		rrtr.SetNtp(TimeMicrosToNtp(clock->TimeInMicroseconds()));
		xr.SetRrtr(rrtr);
//	}

//  for (const rtcp::ReceiveTimeInfo& rti : ctx.feedback_state_.last_xr_rtis) {
//	xr->AddDlrrItem(rti);
//  }

	bFlag = xr.Create((uint8_t *) buffer, &bufferSize, MTU);

	LogAync(LOG_DEBUG, "RtpSession::SendRtcpXr( "
			"this : %p, "
			"bufferSize : %u, "
			"ntp : %llu "
			")", this, bufferSize, rrtr.ntp().ToMs());

	if (bFlag) {
		unsigned int pktSize = (unsigned int) bufferSize;
		bFlag = SendRtcpPacket((void *) buffer, pktSize);
	}

	return bFlag;
}

bool RtpSession::SendRtcpRR() {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;
	char tmp[MTU];
	unsigned int pktSize = 0;
//
//	RtcpPacketCommon pkt = {0};
//	pkt.header.version = 2;
//	pkt.header.p = 0;
//	pkt.header.rc = 1;
//	pkt.header.pt = RtcpPayloadTypeRR;
//	pkt.header.length = 1 + 2 * 6; // (1 * ssrc(32bit)) + (2 * RR(6 * 32bit))
//	pkt.header.length = htons(pkt.header.length);
//	pkt.ssrc = htonl(RTCP_SSRC);
//	memcpy(tmp, (void *)&pkt, sizeof(RtcpPacketCommon));
//	pktSize += sizeof(RtcpPacketCommon);
//
//	// Audio RR
//	RtcpPacketRR rr_audio = {0};
//	rr_audio.media_ssrc = htonl(mAudioSSRC);
//	/**
//	 * 丢包率: RTP包丢失个数 / 期望接收的RTP包个数
//	 * RTP包丢失个数 = 期望接收的RTP包个数 - 实际收到的RTP包个数
//	 * 期望接收的RTP包个数 = 当前最大sequence - 上次最大sequence
//	 * 实际收到的RTP包个数 = 正常有序RTP包 + 重传包
//	 */
//	unsigned int expected = mAudioMaxSeq - mAudioMaxSeqLast;
//	unsigned int actual = mAudioTotalRecvPacket - mAudioTotalRecvPacketLast;
//	mAudioTotalRecvPacketLast = mAudioTotalRecvPacket;
//	rr_audio.fraction = 256 * 1.0f * (expected - actual) / expected;
//	rr_audio.packet_lost = actual;
//	rr_audio.max_seq = htons(mAudioMaxSeq);
//	/**
//	 * 抖动: 一对数据包在接收端与发送端的数据包时间间距之差
//	 * R: RTP包接收的时间戳
//	 * S: RTP包发送的时间戳
//	 * 抖动(i, j) = D(i, j) = |(Rj - Ri) - (Sj - Si)| = |(Rj - Sj) - (Ri - Si)|
//	 * 抖动函数 J(i) = J(i - 1) + (|D(i - 1, i)| - J(i - 1)) / 16
//	 *
//	 * 暂时先不计算, 需要用到RTP扩展头 jitter_q4_transmission_time_offset_
//	 */
//	rr_audio.jitter = 0;
//
//	rr_audio.lsr = mAudioLSR;
//	rr_audio.dlsr = mAudioDLSR;
//	memcpy(tmp + pktSize, (void *)&rr_audio, sizeof(RtcpPacketRR));
//	pktSize += sizeof(RtcpPacketRR);
//
//	// Video RR
//	RtcpPacketRR rr_video = {0};
//	rr_video.media_ssrc = htonl(mVideoSSRC);
//	expected = mVideoMaxSeq - mVideoMaxSeqLast;
//	actual = mVideoTotalRecvPacket - mVideoTotalRecvPacketLast;
//	mVideoTotalRecvPacketLast = mVideoTotalRecvPacket;
//	rr_video.fraction = 256 * 1.0f * (expected - actual) / expected;
//	rr_video.packet_lost = actual;
//	rr_video.max_seq = htons(mVideoMaxSeq);
//	rr_video.jitter = 0;
//	rr_video.lsr = mVideoLSR;
//	rr_video.dlsr = mVideoDLSR;
//	memcpy(tmp + pktSize, (void *)&rr_video, sizeof(RtcpPacketRR));
//	pktSize += sizeof(RtcpPacketRR);

	bFlag = SendRtcpPacket((void *) tmp, pktSize);

	return bFlag;
}

void RtpSession::UpdateStreamInfo(const RtpPacket *pkt, uint64_t recvTime) {
	unsigned short seq = pkt->sequence_number_;
	uint32_t ts = pkt->timestamp_;
	uint32_t ssrc = pkt->ssrc_;

	if (IsAudioPkt(ssrc)) {
		if (mAudioMaxSeq == 0) {
			mAudioMaxSeq = seq;
		}
		if (mAudioMaxTimestamp == 0) {
			mAudioMaxTimestamp = ts;
		}

		LogAync(LOG_DEBUG, "RtpSession::UpdateStreamInfo( "
				"this : %p, "
				"[Audio], "
				"media_ssrc : 0x%08x(%u), "
				"seq : %u, "
				"ts : %u, "
				"recvTime : %lld, "
				"mAudioMaxSeq : %u, "
				"mAudioMaxTimestamp : %u, "
				"mAudioTotalRecvPacket : %u "
				")", this, ssrc, ssrc, seq, ts, recvTime, mAudioMaxSeq,
				mAudioMaxTimestamp, mAudioTotalRecvPacket);

		// 更新丢包信息
		UpdateLossPacket(ssrc, seq, mAudioMaxSeq, ts, mAudioMaxTimestamp);
		// 更新音频时间戳和帧号
		mAudioMaxSeq = MAX(mAudioMaxSeq, seq);
		mAudioMaxTimestamp = MAX(mAudioMaxTimestamp, ts);
		// 更新收到的数据包
		mAudioTotalRecvPacket++;

	} else {
		if (mVideoMaxSeq == 0) {
			mVideoMaxSeq = seq;
		}
		if (mVideoMaxTimestamp == 0) {
			mVideoMaxTimestamp = ts;
		}
		if (mVideoPLITimestamp == 0) {
			mVideoPLITimestamp = ts;
		}
		// 视频帧间隔秒数
		double deltaSeconds = (ts - mVideoPLITimestamp) / 90000.0;
		deltaSeconds = (deltaSeconds > 0) ? deltaSeconds : -deltaSeconds;
		bool sendPLI = false;

		LogAync(LOG_DEBUG, "RtpSession::UpdateStreamInfo( "
				"this : %p, "
				"[Video], "
				"media_ssrc : 0x%08x(%u), "
				"seq : %u, "
				"ts : %u, "
				"recvTime : %lld, "
				"mVideoMaxSeq : %u, "
				"mVideoMaxTimestamp : %u, "
				"mVideoTotalRecvPacket : %u, "
				"deltaSeconds : %f "
				")", this, ssrc, ssrc, seq, ts, recvTime, mVideoMaxSeq,
				mVideoMaxTimestamp, mVideoTotalRecvPacket, deltaSeconds);

//		mRemoteEstimatorProxy.RecvRtpPacket(recvTime, pkt);
//		uint8_t buffer[MTU];
//		size_t bufferSize = 0;
//		mRemoteEstimatorProxy.SendPeriodicFeedbacks(buffer, &bufferSize, MTU);

		/**
		 * 临时方案刷新关键帧, 正常应该根据[丢包/延迟/解码情况]判断是否请求关键帧
		 * 1.每2秒
		 */
		if (pkt->marker_) {
			++mVideoFrameCount;
			if (deltaSeconds > 1) {
				mVideoPLITimestamp = ts;
				sendPLI = true;
//				// 强制刷新一次视频信息(包含视频[宽高/关键帧])
//				SendRtcpFIR(ssrc);
				// 强制刷新一次关键帧
				SendRtcpPLI(ssrc);
				// 发送Remb
//				SendRtcpRemb(ssrc, 500000);
				SendRtcpXr();
			}
		}

		if (!sendPLI) {
			// 更新丢包信息
			UpdateLossPacket(ssrc, seq, mVideoMaxSeq, ts, mVideoMaxTimestamp);
		}

		// 更新视频时间戳和帧号
		mVideoMaxSeq = MAX(mVideoMaxSeq, seq);
		mVideoMaxTimestamp = MAX(mVideoMaxTimestamp, ts);
		// 更新收到的数据包
		mVideoTotalRecvPacket++;
	}
}

bool RtpSession::UpdateLossPacket(unsigned int ssrc, unsigned int seq,
		unsigned int lastMaxSeq, unsigned int ts, unsigned int lastMaxTs) {
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
	int rtpLostSize = (int) (seq - (lastMaxSeq + 1));
	if (rtpLostSize > 0) {
		bFlag = true;

		if (rtpLostSize < 120) {
			// 每个int可以支持最多17个seq
			int quotient = rtpLostSize / 17;
			int remainder = (rtpLostSize % 17);
			int nackItemTotal = (remainder == 0) ? quotient : (quotient + 1);

			// 开始帧号
			int rtpStartSeq = lastMaxSeq + 1;
			// 剩余长度
			int rtpLastCount = rtpLostSize;

			LogAync(LOG_INFO, "RtpSession::UpdateLossPacket( "
					"this : %p, "
					"[%s], "
					"[Packet Loss], "
					"media_ssrc : 0x%08x(%u), "
					"rtpStartSeq : %u, "
					"rtpLostSize : %d, "
					"nackItemTotal : %d "
					")", this, PktTypeDesc(ssrc).c_str(), ssrc, ssrc,
					rtpStartSeq, rtpLostSize, nackItemTotal);

			SendRtcpNack(ssrc, rtpStartSeq, rtpLostSize);
		} else {
			// 丢包太多, 强刷关键帧
			LogAync(LOG_NOTICE, "RtpSession::UpdateLossPacket( "
					"this : %p, "
					"[%s], "
					"[Packet Loss Too Many], "
					"media_ssrc : 0x%08x(%u), "
					"rtpLostSize : %u, "
					"lastMaxSeq : %u, "
					"seq : %u "
					""
					")", this, PktTypeDesc(ssrc).c_str(), ssrc, ssrc,
					rtpLostSize, lastMaxSeq, seq);

			if (!IsAudioPkt(ssrc)) {
				SendRtcpPLI(ssrc);
			}
		}
	}

	return bFlag;
}

void RtpSession::UpdateStreamInfoWithRtcp(const void *pkt,
		unsigned int pktSize) {
	CommonHeader rtcp_block;
	const uint8_t* packet_begin = (const uint8_t*) pkt;
	const uint8_t* packet_end = (const uint8_t*) (pkt + pktSize);

//	Arithmetic ari;
//	string hex = ari.AsciiToHexWithSep((const char *)packet_begin, pktSize);
//	LogAync(
//			LOG_DEBUG,
//			"RtpSession::UpdateStreamInfoWithRtcp( "
//			"this : %p, "
//			"pktSize : %u, "
//			"hex : %s "
//			")",
//			this,
//			pktSize,
//			hex.c_str()
//			);

	for (const uint8_t* next_block = packet_begin; next_block != packet_end;
			next_block = rtcp_block.NextPacket()) {
		int remaining_blocks_size = packet_end - next_block;
		RTC_CHECK_GT(remaining_blocks_size, 0);
//		LogAync(
//				LOG_DEBUG,
//				"RtpSession::UpdateStreamInfoWithRtcp( "
//				"this : %p, "
//				"remaining_blocks_size : %u "
//				")",
//				this,
//				remaining_blocks_size
//				);
		if (!rtcp_block.Parse(next_block, remaining_blocks_size)) {
			if (next_block == packet_begin) {
				// Failed to parse 1st header, nothing was extracted from this packet.
//				RTC_LOG(LS_WARNING) << "Incoming invalid RTCP packet";
				return;
			}
//			++num_skipped_packets_;
			break;
		}

		LogAync(LOG_DEBUG, "RtpSession::UpdateStreamInfoWithRtcp( "
				"this : %p, "
				"type : %u, "
				"payload_size_bytes : %u "
				")", this, rtcp_block.type(),
				rtcp_block.payload_size_bytes());

		switch (rtcp_block.type()) {
		case SenderReport::kPacketType:
			HandleSenderReport(rtcp_block);
			break;
		case ReceiverReport::kPacketType:
			HandleReceiverReport(rtcp_block);
			break;
		case Sdes::kPacketType:
			HandleSdes(rtcp_block);
			break;
		case ExtendedReports::kPacketType:
			HandleXr(rtcp_block);
			break;
		default:
			break;
		};
	}
//	RtcpPayloadType type = (RtcpPayloadType)((RtcpPacketCommon *)pkt)->header.pt;
//	uint32_t ssrc = ntohl(((RtcpPacketCommon *)pkt)->ssrc);
//
//	switch (type) {
//	case RtcpPayloadTypeSR:{
//		RtcpPacketSR *sr = (RtcpPacketSR *)pkt;
//		uint32_t ntp_ts_most = ntohl(sr->ntp_ts_most);
//		uint32_t ntp_ts_least = ntohl(sr->ntp_ts_least);
//		uint32_t rtp_ts = ntohl(sr->rtp_ts);
//		uint32_t sender_packet_count = ntohl(sr->sender_packet_count);
//		uint32_t sender_octet_count = ntohl(sr->sender_octet_count);
//
//		NtpTime lsr = NtpTime(ntp_ts_most, ntp_ts_least);
//		NtpTime now = RealTimeClock::CurrentNtpTime();
//
//		uint32_t seconds = now.seconds() - lsr.seconds();
//		uint32_t fractions = 0;
//		if ( now.fractions() < lsr.fractions() ) {
//			fractions = lsr.fractions() - now.fractions();
//			seconds--;
//		} else {
//			fractions = now.fractions() - lsr.fractions();
//		}
//
//		NtpTime dlsr = NtpTime(seconds, fractions);
//		// 计算RR中的LSR
//		uint32_t rr_lsr = ((lsr.seconds() & 0x0000FFFF) << 16) + ((lsr.fractions() & 0xFFFF0000) >> 16);
//		// 计算RR中的DLSR
//		uint32_t rr_dlsr = ((dlsr.seconds() & 0x0000FFFF) << 16) + ((dlsr.fractions() & 0xFFFF0000) >> 16);
//		uint64_t rr_dlsr_ms = dlsr.ToMs();
//
//		if ( IsAudioPkt(ssrc) ) {
//			mAudioLSR = rr_lsr;
//			mAudioLSR = rr_dlsr;
//		} else {
//			mVideoLSR = rr_lsr;
//			mVideoDLSR = rr_dlsr;
//		}
//
//		LogAync(
//				LOG_DEBUG,
//				"RtpSession::UpdateStreamInfoWithRtcp( "
//				"this : %p, "
//				"[%s], "
//				"[Recv SR], "
//				"media_ssrc : 0x%08x(%u), "
//				"ntp_ts_most : 0x%08x, "
//				"ntp_ts_least : 0x%08x, "
//				"rtp_ts : %u, "
//				"sender_packet_count : %u, "
//				"sender_octet_count : %u, "
//				"rr_lsr : 0x%08x, "
//				"rr_dlsr : 0x%08x(%llums) "
//				")",
//				this,
//				PktTypeDesc(ssrc).c_str(),
//				ssrc,
//				ssrc,
//				ntp_ts_most,
//				ntp_ts_least,
//				rtp_ts,
//				sender_packet_count,
//				sender_octet_count,
//				rr_lsr,
//				rr_dlsr,
//				rr_dlsr_ms
//				);
//
//	}break;
//	default:break;
//	}
}

void RtpSession::HandleSenderReport(const CommonHeader& rtcp_block) {
	SenderReport sr;
	if (!sr.Parse(rtcp_block)) {
		return;
	}

	const uint32_t sender_ssrc = sr.sender_ssrc_;
	// Have I received RTP packets from this party?
	mRemoteSenderNtpTime = sr.ntp();
	mRemoteSenderRtpTime = sr.rtp_timestamp();
//    last_received_sr_ntp_ = TimeMicrosToNtp(clock_->TimeInMicroseconds());
	LogAync(LOG_DEBUG, "RtpSession::HandleSenderReport( "
			"this : %p, "
			"[Recv Sr], "
			"sender_ssrc : 0x%08x(%u), "
			"ts : %u, "
			"seconds : %u, "
			"fractions : %u, "
			"sender_packet_count : %u, "
			"sender_octet_count : %u, "
			"report_blocks.count : %u "
			")", this, sender_ssrc, sender_ssrc, mRemoteSenderRtpTime,
			mRemoteSenderNtpTime.seconds(), mRemoteSenderNtpTime.fractions(),
			sr.sender_packet_count(), sr.sender_octet_count(),
			sr.report_blocks().size());

	for (const ReportBlock& report_block : sr.report_blocks()) {
		HandleReportBlock(report_block);
	}
}

void RtpSession::HandleReceiverReport(const CommonHeader& rtcp_block) {
	ReceiverReport rr;
	if (!rr.Parse(rtcp_block)) {
		return;
	}

	const uint32_t sender_ssrc = rr.sender_ssrc_;
	LogAync(LOG_DEBUG, "RtpSession::HandleReceiverReport( "
			"this : %p, "
			"[Recv Rr], "
			"sender_ssrc : 0x%08x(%u), "
			"report_blocks.count : %u "
			")", this, sender_ssrc, sender_ssrc, rr.report_blocks().size());
//  UpdateTmmbrRemoteIsAlive(remote_ssrc);
//
//  packet_information->packet_type_flags |= kRtcpRr;
//
	for (const ReportBlock& report_block : rr.report_blocks()) {
		HandleReportBlock(report_block);
	}
}

void RtpSession::HandleReportBlock(const ReportBlock& report_block) {
	uint32_t media_ssrc = report_block.source_ssrc();
	uint8_t fraction_lost = report_block.fraction_lost();
	int32_t cumulative_lost_signed = report_block.cumulative_lost_signed();
	uint32_t extended_highest_sequence_number =
			report_block.extended_high_seq_num();
	uint32_t jitter = report_block.jitter();
	uint32_t delay_since_last_sr = report_block.delay_since_last_sr();
	uint32_t last_sr = report_block.last_sr();

//	int64_t rtt_ms = 0;
//	uint32_t send_time_ntp = report_block.last_sr();
//	uint32_t delay_ntp = 0;
//	if (send_time_ntp != 0) {
//		delay_ntp = report_block.delay_since_last_sr();
////		// Local NTP time.
////		uint32_t receive_time_ntp = CompactNtp(
////				TimeMicrosToNtp(clock_->TimeInMicroseconds()));
////
////		// RTT in 1/(2^16) seconds.
////		uint32_t rtt_ntp = receive_time_ntp - delay_ntp - send_time_ntp;
////		// Convert to 1/1000 seconds (milliseconds).
////		rtt_ms = CompactNtpRttToMs(rtt_ntp);
//	}

	LogAync(LOG_DEBUG, "RtpSession::HandleReportBlock( "
			"this : %p, "
			"[%s], [Recv Report], "
			"media_ssrc : 0x%08x(%u), "
			"fraction_lost : %u, "
			"cumulative_lost_signed : %d, "
			"extended_highest_sequence_number : %u, "
			"jitter : %u, "
			"delay_since_last_sr : %u, "
			"last_sr : %u "
			")", this, PktTypeDesc(media_ssrc).c_str(), media_ssrc, media_ssrc,
			fraction_lost, cumulative_lost_signed,
			extended_highest_sequence_number, jitter, delay_since_last_sr,
			last_sr);
}

void RtpSession::HandleSdes(const CommonHeader& rtcp_block) {
	Sdes sdes;
	if (!sdes.Parse(rtcp_block)) {
//    ++num_skipped_packets_;
		return;
	}

//	for (const Sdes::Chunk& chunk : sdes.chunks()) {
//	}
}

void RtpSession::HandleXr(const CommonHeader& rtcp_block) {
	ExtendedReports xr;
	if (!xr.Parse(rtcp_block)) {
//    ++num_skipped_packets_;
		return;
	}

	const uint32_t sender_ssrc = xr.sender_ssrc_;
	LogAync(LOG_DEBUG, "RtpSession::HandleXr( "
			"this : %p, "
			"[Recv Xr], "
			"sender_ssrc : 0x%08x(%u), "
			")", this, sender_ssrc, sender_ssrc);
//  if (xr.rrtr())
//    HandleXrReceiveReferenceTime(xr.sender_ssrc(), *xr.rrtr());
//
//  for (const rtcp::ReceiveTimeInfo& time_info : xr.dlrr().sub_blocks())
//    HandleXrDlrrReportBlock(time_info);
//
//  if (xr.target_bitrate()) {
//    HandleXrTargetBitrate(xr.sender_ssrc(), *xr.target_bitrate(),
//                          packet_information);
//  }
}

string RtpSession::PktTypeDesc(unsigned int ssrc) {
	return IsAudioPkt(ssrc) ? "Audio" : "Video";
}

bool RtpSession::IsAudioPkt(unsigned int ssrc) {
	return (ssrc == mAudioSSRC);
}

bool RtpSession::SimPktLost(unsigned int media_ssrc, unsigned int seq) {
	bool bFlag = true;
	if (mbSimLost) {
		if (IsAudioPkt(media_ssrc)) {
			if (mbAudioAbandonning) {
				if (++mAudioAbandonCount <= mAudioAbandonTotal) {
					bFlag = false;
				} else {
					mbAudioAbandonning = false;
					mAudioAbandonTotal = 0;
					mAudioAbandonCount = 0;
				}
			} else {
				int check = rand() % mAudioLostSeed;
				if (check == 0) {
					// 模拟丢包
					mbAudioAbandonning = true;
					mAudioAbandonTotal = rand() % mAudioLostSize;
					mAudioAbandonCount = 0;
					bFlag = false;

					LogAync(LOG_NOTICE, "RtpSession::SimPktLost( "
							"this : %p, "
							"[%s], "
							"media_ssrc : 0x%08x(%u), "
							"seq : %u, "
							"total : %u "
							")", this, PktTypeDesc(media_ssrc).c_str(),
							media_ssrc, media_ssrc, seq, mAudioAbandonTotal);
				}
			}
		} else {
			if (mbVideoAbandonning) {
				if (++mVideoAbandonCount <= mVideoAbandonTotal) {
					bFlag = false;
				} else {
					mbVideoAbandonning = false;
					mVideoAbandonTotal = 0;
					mVideoAbandonCount = 0;
				}
			} else {
				int check = rand() % mVideoLostSeed;
				if (check == 0) {
					// 模拟丢包
					mbVideoAbandonning = true;
					mVideoAbandonTotal = rand() % mVideoLostSize;
					mVideoAbandonCount = 0;
					bFlag = false;

					LogAync(LOG_NOTICE, "RtpSession::SimPktLost( "
							"this : %p, "
							"[%s], "
							"media_ssrc : 0x%08x(%u), "
							"seq : %u, "
							"total : %u "
							")", this, PktTypeDesc(media_ssrc).c_str(),
							media_ssrc, media_ssrc, seq, mVideoAbandonTotal);
				}
			}
		}
	}
	return bFlag;
}

unsigned int RtpSession::GetRtpSSRC(void *pkt, unsigned int& pktSize) {
	int ssrc = 0;
	RtpHeader *header = (RtpHeader *) pkt;
	ssrc = ntohl(header->ssrc);
	return ssrc;
}

unsigned int RtpSession::GetRtcpSSRC(void *pkt, unsigned int& pktSize) {
	int ssrc = 0;
//	RtcpPacketCommon *header = (RtcpPacketCommon *)pkt;
//	ssrc = ntohl(header->ssrc);
	return ssrc;
}

bool RtpSession::IsRtp(const char *frame, unsigned len) {
	bool bFlag = false;
	if (len > sizeof(RtpHeader)) {
		RtpHeader *header = (RtpHeader *) frame;
		bFlag =
				((header->pt < 64) || ((header->pt >= 96) && (header->pt < 200)));
	}
	return bFlag;
}

bool RtpSession::IsRtcp(const char *frame, unsigned len) {
	bool bFlag = false;
	if (len > sizeof(RtcpHeader)) {
		RtcpHeader *header = (RtcpHeader *) frame;
		bFlag = ((header->pt >= 200) && (header->pt <= 210));
	}
	return bFlag;
}

} /* namespace mediaserver */
