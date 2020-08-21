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
#include <rtp/base/time_utils.h>
#include <rtp/include/time_util.h>
#include <rtp/include/rtp_rtcp_config.h>

#include <include/CommonHeader.h>

#include <common/LogManager.h>
#include <common/Math.h>
#include <common/CommonFunc.h>
#include <common/Arithmetic.h>
#include <common/Math.h>

// libsrtp
#include <srtp.h>
#include <srtp_priv.h>

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
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

constexpr int kRembSendIntervalMs = 200;
constexpr int kRRSendIntervalMs = 5000;

static unsigned int gMaxPliSeconds = 3;
static bool gSimLost = false;
static unsigned int gAudioLostSeed = 300;
static unsigned int gAudioLostSize = 150;
static unsigned int gVideoLostSeed = 100;
static unsigned int gVideoLostSize = 50;
static bool gAutoBitrate = true;
static unsigned int gVideoMinBitrate = 200 * 1000;
static unsigned int gVideoMaxBitrate = 1000 * 1000;
void RtpSession::SetGobalParam(
		unsigned int maxPliSeconds,
		bool autoBitrate, unsigned int videoMinBitrate, unsigned int videoMaxBitrate,
		bool simLost) {
	// 关键帧最大间隔
	gMaxPliSeconds = maxPliSeconds;
	// 自适应码率
	gAutoBitrate = autoBitrate;
	// 自适应码率模式下, 视频最小码率
	gVideoMinBitrate = videoMinBitrate;
	// 自适应码率模式下, 视频最大码率
	gVideoMaxBitrate = videoMaxBitrate;
	// 模拟丢包
	gSimLost = simLost;
}

RtpSession::RtpSession() :
		mClientMutex(KMutex::MutexType_Recursive),
		nack_module_(Clock::GetRealTimeClock()),
		rbe_module_(this, Clock::GetRealTimeClock()),
		rs_module_(Clock::GetRealTimeClock()) {
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

	mVideoExtensionMap.Clear();
	mAudioExtensionMap.Clear();

	// 统计RTCP中的 RTT/LOSS
	remote_sender_rtp_time_ = 0;
	remote_sender_ntp_time_.Reset();
	last_received_sr_ntp_.Reset();
	xr_rr_rtt_ms_ = 0;
	xr_last_send_ms_ = 0;
	last_rr_send_time_ = 0;

	last_remb_time_ms_ = 0;
	last_send_bitrate_bps_ = 0;
	bitrate_bps_ = 0;
	max_bitrate_bps_ = gVideoMaxBitrate;

	// NACK模块
	nack_module_.Reset();
	// RBE模块
	rbe_module_.SetMinBitrate(gVideoMinBitrate);
	rbe_module_.Reset();
	// 接收统计模块
	rs_module_.Reset();
	rs_module_.SetMaxReorderingThreshold(kDefaultMaxReorderingThreshold);
}

void RtpSession::SetRtpSender(SocketSender *sender) {
	mpRtpSender = sender;
}

void RtpSession::SetRtcpSender(SocketSender *sender) {
	mpRtcpSender = sender;
}

void RtpSession::SetVideoSSRC(unsigned int ssrc) {
	mVideoSSRC = ssrc;
	rs_module_.EnableRetransmitDetection(mVideoSSRC, true);
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
	Reset();
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
	gSimLost = bSimLost;
	gAudioLostSeed = audioLostSeed;
	gAudioLostSize = audioLostSize;
	gVideoLostSeed = videoLostSeed;
	gVideoLostSize = videoLostSize;
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
	mpRecvPolicy->window_size = 1024;
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

			RtpHeader *h = (RtpHeader*) pkt;
			uint32_t ssrc = ntohl(h->ssrc);
			uint16_t seq = ntohs(h->seq);
			uint32_t ts = ntohl(h->ts);
//			LogAync(LOG_DEBUG, "RtpSession::RecvRtpPacket( "
//					"this : %p, "
//					"[%s], "
//					"media_ssrc : 0x%08x(%u), "
//					"x : %u, "
//					"seq : %u, "
//					"ts : %u, "
//					"recvTime : %lld, "
//					"size : %d"
//					"%s"
//					")", this,
//					PktTypeDesc(ssrc).c_str(),
//					ssrc,
//					ssrc,
//					h->x,
//					seq,
//					ts,
//					recvTime,
//					size,
//					h->m? ", [Mark] " : " ");

			// 模拟丢包
			bFlag = SimPktLost(ssrc, seq);
			if (bFlag && mpRecvSrtpCtx) {
				srtp_err_status_t status = srtp_unprotect(mpRecvSrtpCtx, pkt,
						(int *) &pktSize);
				bFlag = (status == srtp_err_status_ok);
				if (bFlag) {
//					RtpPacket rtpPkt;
					RtpPacketReceived rtpPkt(&mVideoExtensionMap);
					rtpPkt.set_arrival_time_ms(recvTime);
					// 更新频率
					if (!IsAudioPkt(ssrc)) {
						rtpPkt.set_payload_type_frequency(kVideoPayloadTypeFrequency);
					}

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
								"payloadSize : %d"
								"%s"
								")", this, PktTypeDesc(header.ssrc).c_str(),
								header.ssrc, header.ssrc,
								rtpPkt.has_extension_,
								header.sequenceNumber,
								header.timestamp,
								header.extension.GetAbsoluteSendTimestamp().ms(),
								recvTime,
								rtpPkt.payload_size() + rtpPkt.padding_size(),
								header.markerBit ? ", [Mark] " : " ");
						if (bFlag) {
							// 更新媒体流信息
							UpdateStreamInfo(&rtpPkt, recvTime, header);
						}
					} else {
						LogAync(LOG_DEBUG, "RtpSession::RecvRtpPacket( "
								"this : %p, "
								"[%s], [Parse Rtp Packet Error], "
								"media_ssrc : 0x%08x(%u), "
								"x : %u, "
								"seq : %u, "
								"ts : %u, "
								"recvTime : %lld, "
								"size : %d"
								"%s"
								")", this, PktTypeDesc(ssrc).c_str(), ssrc,
								ssrc, h->x, seq, ts, recvTime, size,
								h->m ? ", [Mark] " : " ");
					}
				} else {
					LogAync(LOG_DEBUG, "RtpSession::RecvRtpPacket( "
							"this : %p, "
							"[%s], [Parse Rtp Packet Error %d], "
							"media_ssrc : 0x%08x(%u), "
							"x : %u, "
							"seq : %u, "
							"ts : %u, "
							"recvTime : %lld, "
							"size : %d"
							"%s"
							")", this, PktTypeDesc(ssrc).c_str(), status, ssrc,
							ssrc, h->x, seq, ts, recvTime, size,
							h->m ? ", [Mark] " : " ");
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

    if ( !bFlag ) {
    	RtpHeader *header = (RtpHeader *)pkt;

		LogAync(
				LOG_WARNING,
				"RtpSession::SendRtpPacket( "
				"this : %p, "
				"status : %d, "
				"sendSize : %d, "
				"pktSize : %d, "
				"seq : %u, "
				"timestamp : %u "
				")",
				this,
				status,
				sendSize,
				pktSize,
				ntohs(header->seq),
				ntohl(header->ts)
				);
    }

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

bool RtpSession::SendRtcpPli(unsigned int media_ssrc) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;

	uint8_t buffer[MTU];
	size_t bufferSize = 0;

	Pli pli;
	pli.sender_ssrc_ = RTCP_SSRC;
	pli.media_ssrc_ = media_ssrc;
	bFlag = pli.Create((uint8_t *) buffer, &bufferSize, MTU);

	LogAync(LOG_DEBUG, "RtpSession::SendRtcpPli( "
			"this : %p, "
			"[%s], "
			"media_ssrc : 0x%08x(%u) "
			")", this, PktTypeDesc(media_ssrc).c_str(), media_ssrc, media_ssrc
			);

	if (bFlag) {
		unsigned int pktSize = (unsigned int) bufferSize;
		bFlag = SendRtcpPacket((void *) buffer, pktSize);
	}

	return bFlag;
}

bool RtpSession::SendRtcpFir(unsigned int media_ssrc) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;

	uint8_t buffer[MTU];
	size_t bufferSize = 0;

	Fir fir;
	fir.sender_ssrc_ = RTCP_SSRC;
	fir.AddRequestTo(media_ssrc, ++mFirSeq);
	bFlag = fir.Create((uint8_t *) buffer, &bufferSize, MTU);

	LogAync(LOG_DEBUG, "RtpSession::SendRtcpFir( "
			"this : %p, "
			"[%s], "
			"media_ssrc : 0x%08x(%u), "
			"bufferSize : %u "
			")", this, PktTypeDesc(media_ssrc).c_str(), media_ssrc, media_ssrc,
			bufferSize);

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
			"media_ssrc_ : 0x%08x(%u), "
			"bufferSize : %u, "
			"start : %u, "
			"size : %u "
			")", this, PktTypeDesc(media_ssrc).c_str(), media_ssrc, media_ssrc,
			bufferSize, start, size);

	if (bFlag) {
		unsigned int pktSize = (unsigned int) bufferSize;
		bFlag = SendRtcpPacket((void *) buffer, pktSize);
	}

	return bFlag;
}

bool RtpSession::SendRtcpNack(unsigned int media_ssrc,
		const std::vector<uint16_t> &nack_batch) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;

	uint8_t buffer[MTU];
	size_t bufferSize = 0;

	Nack nack;
	nack.sender_ssrc_ = RTCP_SSRC;
	nack.media_ssrc_ = media_ssrc;
	nack.SetPacketIds(nack_batch);
	bFlag = nack.Create(buffer, &bufferSize, MTU);

	string nacks = "";
	for (vector<uint16_t>::const_iterator itr = nack_batch.begin();
			itr != nack_batch.end(); itr++) {
		nacks += to_string(*itr);
		nacks += ",";
	}
	if (nacks.length() > 0) {
		nacks = nacks.substr(0, nacks.length() - 1);
	}

	LogAync(LOG_DEBUG, "RtpSession::SendRtcpNack( "
			"this : %p, "
			"[%s], "
			"media_ssrc_ : 0x%08x(%u), "
			"nack_batch_size : %u, "
			"nack_batch : %s "
			")", this, PktTypeDesc(media_ssrc).c_str(), media_ssrc, media_ssrc,
			nack_batch.size(), nacks.c_str());

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
			")", this, PktTypeDesc(media_ssrc).c_str(), media_ssrc, media_ssrc,
			bufferSize, bitrate);

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

	Rrtr rrtr;
	Clock *clock = Clock::GetRealTimeClock();
	NtpTime ntp = TimeMicrosToNtp(clock->TimeInMicroseconds());
	rrtr.SetNtp(ntp);
	xr.SetRrtr(rrtr);

//  for (const rtcp::ReceiveTimeInfo& rti : ctx.feedback_state_.last_xr_rtis) {
//	xr->AddDlrrItem(rti);
//  }

	bFlag = xr.Create((uint8_t *) buffer, &bufferSize, MTU);

	LogAync(LOG_DEBUG, "RtpSession::SendRtcpXr( "
			"this : %p, "
			"bufferSize : %u, "
			"ntp : %u "
			")", this, bufferSize, CompactNtp(rrtr.ntp()));

	if (bFlag) {
		unsigned int pktSize = (unsigned int) bufferSize;
		bFlag = SendRtcpPacket((void *) buffer, pktSize);
	}

	return bFlag;
}

bool RtpSession::SendRtcpRr(const std::vector<rtcp::ReportBlock> &result) {
	bool bFlag = false;

	srtp_err_status_t status = srtp_err_status_fail;

	uint8_t buffer[MTU];
	size_t bufferSize = 0;

	ReceiverReport rr;
	rr.SetSenderSsrc(RTCP_SSRC);
	rr.SetReportBlocks(result);

	bFlag = rr.Create((uint8_t *) buffer, &bufferSize, MTU);

	LogAync(LOG_DEBUG, "RtpSession::SendRtcpRr( "
			"this : %p, "
			"bufferSize : %u "
			")", this, bufferSize);

	if (bFlag) {
		unsigned int pktSize = (unsigned int) bufferSize;
		bFlag = SendRtcpPacket((void *) buffer, pktSize);
	}

	return bFlag;
}

void RtpSession::UpdateStreamInfo(const RtpPacketReceived *rtpPkt, uint64_t recvTime, const RTPHeader& header) {
	uint16_t seq = rtpPkt->sequence_number_;
	uint32_t ts = rtpPkt->timestamp_;
	uint32_t ssrc = rtpPkt->ssrc_;

	if (IsAudioPkt(ssrc)) {
		if (mAudioMaxSeq == 0) {
			mAudioMaxSeq = seq;
		}
		if (mAudioMaxTimestamp == 0) {
			mAudioMaxTimestamp = ts;
		}

		// 更新丢包信息
		UpdateAudioLossPacket(rtpPkt, recvTime);

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

		// 更新接收包统计
		UpdateVideoStatsPacket(rtpPkt, recvTime);
		// 处理丢包逻辑
		UpdateVideoLossPacket(rtpPkt, recvTime);
		// 更新接收端滤波器
		rbe_module_.IncomingPacket(recvTime, rtpPkt->payload_size() + rtpPkt->padding_size(), header);

		// 更新视频时间戳和帧号
		mVideoMaxSeq = MAX(mVideoMaxSeq, seq);
		mVideoMaxTimestamp = MAX(mVideoMaxTimestamp, ts);
		// 更新收到的数据包
		mVideoTotalRecvPacket++;
	}
}

bool RtpSession::UpdateAudioLossPacket(const RtpPacketReceived *pkt,
		uint64_t recvTime) {
	uint16_t seq = pkt->sequence_number_;
	uint32_t ts = pkt->timestamp_;
	uint32_t ssrc = pkt->ssrc_;

	bool bFlag = false;

	LogAync(LOG_DEBUG, "RtpSession::UpdateAudioLossPacket( "
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

	// 音频不处理NACK
//	/**
//	 * 暂时只做简单处理, 直接发送Nack
//	 *
//	 * 正常算法
//	 * 	1.判断丢包队列数量是否过大, 过大则清空丢包队列, 不进行Nack, 直接请求关键帧(PLI)
//	 * 	2.判断丢包队列最大时长(根据RTT计算)是否过大, 过大则清空丢包队列, 不进行Nack, 直接请求关键帧(PLI)
//	 *	3.从(seq) - ((last seq) + 1)的序号开始到seq - 1, 放进Nack队列, 等待重传, 等待时间(根据RTT计算)
//	 *	4.在独立的处理线程处理丢包队, 发送Nack
//	 */
//	bool bFlag = false;
//
//	// 开始产生重传队列
//	int rtpLostSize = (int) (seq - (mAudioMaxSeq + 1));
//	if (rtpLostSize > 0) {
//		bFlag = true;
//
//		if (rtpLostSize < 120) {
//			// 每个int可以支持最多17个seq
//			int quotient = rtpLostSize / 17;
//			int remainder = (rtpLostSize % 17);
//			int nackItemTotal = (remainder == 0) ? quotient : (quotient + 1);
//
//			// 开始帧号
//			int rtpStartSeq = lastMaxSeq + 1;
//			// 剩余长度
//			int rtpLastCount = rtpLostSize;
//
//			LogAync(LOG_INFO, "RtpSession::UpdateAudioLossPacket( "
//					"this : %p, "
//					"[%s], "
//					"[Packet Loss], "
//					"media_ssrc : 0x%08x(%u), "
//					"rtpStartSeq : %u, "
//					"rtpLostSize : %d, "
//					"nackItemTotal : %d "
//					")", this, PktTypeDesc(ssrc).c_str(), ssrc, ssrc,
//					rtpStartSeq, rtpLostSize, nackItemTotal);
//
//			SendRtcpNack(ssrc, rtpStartSeq, rtpLostSize);
//		} else {
//			// 丢包太多
//			LogAync(LOG_NOTICE, "RtpSession::UpdateAudioLossPacket( "
//					"this : %p, "
//					"[%s], "
//					"[Packet Loss Too Many], "
//					"media_ssrc : 0x%08x(%u), "
//					"rtpLostSize : %u, "
//					"lastMaxSeq : %u, "
//					"seq : %u "
//					""
//					")", this, PktTypeDesc(ssrc).c_str(), ssrc, ssrc,
//					rtpLostSize, lastMaxSeq, seq);
//		}
//	}

	return bFlag;
}

bool RtpSession::UpdateVideoStatsPacket(const RtpPacketReceived *rtpPkt, uint64_t recvTime) {
	bool bFlag = true;

	uint16_t seq = rtpPkt->sequence_number_;
	uint32_t ts = rtpPkt->timestamp_;
	uint32_t ssrc = rtpPkt->ssrc_;

	// 更新接收包统计
	rs_module_.OnRtpPacket(*rtpPkt);

    StreamStatistician* ss = rs_module_.GetStatistician(ssrc);
	if ( ss ) {
		int64_t now_ms = TimeMillis();
		if ( last_rr_send_time_ == 0 ) {
			last_rr_send_time_ = now_ms;
		}

		if ( now_ms - last_rr_send_time_ > kRRSendIntervalMs ) {
			int index = -1;
			int step = 0;
			std::vector<rtcp::ReportBlock> result = rs_module_.RtcpReportBlocks(RTCP_MAX_REPORT_BLOCKS);
		    for (auto& report_block : result) {
		    	if ( report_block.source_ssrc() == ssrc ) {
		    		index = step;
		    		break;
		    	}
		    	step++;
		    }

		    double fractionLostInPercent = index > -1?(result[index].fraction_lost() * 100.0 / 255.0):0;
			LogAync(LOG_INFO, "RtpSession::UpdateVideoStatsPacket( "
					"this : %p, "
					"[%s], "
					"media_ssrc : 0x%08x(%u), "
					"bitrate_est : %u, "
					"bitrate_recv : %u, "
					"extended_highest_sequence_number : %u, "
					"rtt : %" PRId64 ", "
					"packets_lost : %d, "
					"jitter : %u, "
					"fraction_since_last : %u%%(%hhu) "
					")",
					this,
					PktTypeDesc(ssrc).c_str(), ssrc, ssrc,
					last_send_bitrate_bps_,
					ss->BitrateReceived(),
					index > -1?result[index].extended_high_seq_num():0,
					xr_rr_rtt_ms_,
					index > -1?result[index].cumulative_lost_signed():0,
					index > -1?result[index].jitter():0,
					(unsigned int)fractionLostInPercent,
					index > -1?(result[index].fraction_lost()):0
					);

			SendRtcpRr(result);

			last_rr_send_time_ = now_ms;
		}
	}

	return bFlag;
}

bool RtpSession::UpdateVideoLossPacket(const RtpPacketReceived *rtpPkt,
		uint64_t recvTime) {
	uint16_t seq = rtpPkt->sequence_number_;
	uint32_t ts = rtpPkt->timestamp_;
	uint32_t ssrc = rtpPkt->ssrc_;

	bool bFlag = false;

	bool sendPLI = false;
	if (mVideoPLITimestamp == 0) {
		mVideoPLITimestamp = ts;
		sendPLI = true;
	}

	// 视频帧间隔秒数
	double deltaSeconds = (ts - mVideoPLITimestamp) / 90000.0;
	deltaSeconds = (deltaSeconds > 0) ? deltaSeconds : -deltaSeconds;

	LogAync(LOG_DEBUG, "RtpSession::UpdateVideoLossPacket( "
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

	/**
	 * 每gMaxPliSeconds秒, 发送PLI
	 */
	if (deltaSeconds > gMaxPliSeconds) {
		sendPLI = true;
		SendRtcpPli(ssrc);
		mVideoPLITimestamp = ts;
	}

	std::vector<uint16_t> nack_batch;
	bool need_request_keyframe;
	nack_module_.OnReceivedPacket(seq, false, nack_batch,
			need_request_keyframe);

	std::vector<uint16_t> nack_list;
	nack_module_.GetNackList(nack_list);

	LogAync(LOG_DEBUG, "RtpSession::UpdateVideoLossPacket( "
			"this : %p, "
			"[%s], "
			"ssrc : 0x%08x(%u), "
			"nack_batch_size : %u, "
			"nack_list_size : %u, "
			"need_request_keyframe : %s "
			")", this, PktTypeDesc(ssrc).c_str(), ssrc, ssrc, nack_batch.size(),
			nack_list.size(),
			BOOL_2_STRING(need_request_keyframe)
			);

	 if (!nack_batch.empty()) {
		// 请求NACK重传
		SendRtcpNack(ssrc, nack_batch);
	}

	if (sendPLI) {
//		nack_module_.Clear();
	} else if (need_request_keyframe) {
		// 丢包严重, 发送PLI
		SendRtcpPli(ssrc);
		mVideoPLITimestamp = ts;
	}

	// 定时发送RRTR, 计算RTT
	Clock *clock = Clock::GetRealTimeClock();
	int64_t now = clock->TimeInMilliseconds();
	if (xr_last_send_ms_ == 0 || now - xr_last_send_ms_ > 1000) {
		xr_last_send_ms_ = now;
		SendRtcpXr();
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
				")", this, rtcp_block.type(), rtcp_block.payload_size_bytes());

		switch (rtcp_block.type()) {
		case SenderReport::kPacketType:
			HandleRtcpSr(rtcp_block);
			break;
		case ReceiverReport::kPacketType:
			HandleRtcpRr(rtcp_block);
			break;
		case Sdes::kPacketType:
			HandleRtcpSdes(rtcp_block);
			break;
		case ExtendedReports::kPacketType:
			HandleRtcpXr(rtcp_block);
			break;
		default:
			break;
		};
	}
}

void RtpSession::HandleRtcpSr(const CommonHeader& rtcp_block) {
	SenderReport sr;
	if (!sr.Parse(rtcp_block)) {
		return;
	}

	const uint32_t sender_ssrc = sr.sender_ssrc_;
	// Have I received RTP packets from this party?
	remote_sender_ntp_time_ = sr.ntp();
	remote_sender_rtp_time_ = sr.rtp_timestamp();
	Clock *clock = Clock::GetRealTimeClock();
	last_received_sr_ntp_ = TimeMicrosToNtp(clock->TimeInMicroseconds());
	LogAync(LOG_DEBUG, "RtpSession::HandleRtcpSr( "
			"this : %p, "
			"sender_ssrc : 0x%08x(%u), "
			"ts : %u, "
			"remote_sender_ntp_time_ : %llu, "
			"last_received_sr_ntp_ : %llu, "
			"sender_packet_count : %u, "
			"sender_octet_count : %u, "
			"report_blocks.count : %u "
			")", this, sender_ssrc, sender_ssrc, remote_sender_rtp_time_,
			remote_sender_ntp_time_.ToMs(), last_received_sr_ntp_.ToMs(),
			sr.sender_packet_count(), sr.sender_octet_count(),
			sr.report_blocks().size());

	for (const ReportBlock& report_block : sr.report_blocks()) {
		HandleRtcpRb(report_block);
	}
}

void RtpSession::HandleRtcpRr(const CommonHeader& rtcp_block) {
	ReceiverReport rr;
	if (!rr.Parse(rtcp_block)) {
		return;
	}

	const uint32_t sender_ssrc = rr.sender_ssrc_;
	LogAync(LOG_DEBUG, "RtpSession::HandleRtcpRr( "
			"this : %p, "
			"sender_ssrc : 0x%08x(%u), "
			"report_blocks.count : %u "
			")", this, sender_ssrc, sender_ssrc, rr.report_blocks().size());
//  UpdateTmmbrRemoteIsAlive(remote_ssrc);
//
//  packet_information->packet_type_flags |= kRtcpRr;
//
	for (const ReportBlock& report_block : rr.report_blocks()) {
		HandleRtcpRb(report_block);
	}
}

void RtpSession::HandleRtcpRb(const ReportBlock& report_block) {
	uint32_t media_ssrc = report_block.source_ssrc();
	uint8_t fraction_lost = report_block.fraction_lost();
	int32_t cumulative_lost_signed = report_block.cumulative_lost_signed();
	uint32_t extended_highest_sequence_number =
			report_block.extended_high_seq_num();
	uint32_t jitter = report_block.jitter();
	uint32_t delay_since_last_sr = report_block.delay_since_last_sr();
	uint32_t last_sr = report_block.last_sr();

	int64_t rtt_ms = 0;
	uint32_t send_time_ntp = report_block.last_sr();
	uint32_t delay_ntp = 0;
	if (send_time_ntp != 0) {
		delay_ntp = report_block.delay_since_last_sr();
//		// Local NTP time.
//		uint32_t receive_time_ntp = CompactNtp(
//				TimeMicrosToNtp(clock_->TimeInMicroseconds()));
//
//		// RTT in 1/(2^16) seconds.
//		uint32_t rtt_ntp = receive_time_ntp - delay_ntp - send_time_ntp;
//		// Convert to 1/1000 seconds (milliseconds).
//		rtt_ms = CompactNtpRttToMs(rtt_ntp);
	}

	LogAync(LOG_DEBUG, "RtpSession::HandleRtcpRb( "
			"this : %p, "
			"[%s], "
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

void RtpSession::HandleRtcpSdes(const CommonHeader& rtcp_block) {
	Sdes sdes;
	if (!sdes.Parse(rtcp_block)) {
//    ++num_skipped_packets_;
		return;
	}

//	for (const Sdes::Chunk& chunk : sdes.chunks()) {
//	}
}

void RtpSession::HandleRtcpXr(const CommonHeader& rtcp_block) {
	ExtendedReports xr;
	if (!xr.Parse(rtcp_block)) {
//    ++num_skipped_packets_;
		return;
	}

	const uint32_t sender_ssrc = xr.sender_ssrc_;
	LogAync(LOG_DEBUG, "RtpSession::HandleRtcpXr( "
			"this : %p, "
			"sender_ssrc : 0x%08x(%u) "
			")", this, sender_ssrc, sender_ssrc);
//  if (xr.rrtr())
//    HandleRtcpXrReceiveReferenceTime(xr.sender_ssrc(), *xr.rrtr());
//
	for (const rtcp::ReceiveTimeInfo& time_info : xr.dlrr().sub_blocks()) {
		HandleRtcpXrDlrr(time_info);
	}
}

void RtpSession::HandleRtcpXrDlrr(const ReceiveTimeInfo& rti) {
	if (rti.ssrc != RTCP_SSRC) {
		return; // Not to us.
	}

	// The send_time and delay_rr fields are in units of 1/2^16 sec.
	uint32_t send_time_ntp = rti.last_rr;
	// RFC3611, section 4.5, LRR field discription states:
	// If no such block has been received, the field is set to zero.
	if (send_time_ntp == 0) {
		return;
	}

	uint32_t delay_ntp = rti.delay_since_last_rr;
	Clock *clock = Clock::GetRealTimeClock();
	uint32_t now_ntp = CompactNtp(TimeMicrosToNtp(clock->TimeInMicroseconds()));

	uint32_t rtt_ntp = now_ntp - delay_ntp - send_time_ntp;
	xr_rr_rtt_ms_ = CompactNtpRttToMs(rtt_ntp);
	nack_module_.UpdateRtt(xr_rr_rtt_ms_);
	rbe_module_.OnRttUpdate(xr_rr_rtt_ms_, xr_rr_rtt_ms_);

	LogAync(LOG_DEBUG, "RtpSession::HandleRtcpXrDlrr( "
			"this : %p, "
			"source_ssrc : 0x%08x(%u), "
			"now : %u, "
			"lrr : %u, "
			"dlrr : %u, "
			"rtt_ntp : %u, "
			"xr_rr_rtt_ms_ : %llu "
			")", this, rti.ssrc, rti.ssrc, now_ntp, send_time_ntp, delay_ntp,
			rtt_ntp, xr_rr_rtt_ms_);
}

string RtpSession::PktTypeDesc(unsigned int ssrc) {
	return IsAudioPkt(ssrc) ? "Audio" : "Video";
}

bool RtpSession::IsAudioPkt(unsigned int ssrc) {
	return (ssrc == mAudioSSRC);
}

bool RtpSession::IsVideoPkt(unsigned int ssrc) {
	return (ssrc == mVideoSSRC);
}

bool RtpSession::SimPktLost(unsigned int media_ssrc, unsigned int seq) {
	bool bFlag = true;
	if (gSimLost) {
		if (IsAudioPkt(media_ssrc)) {
			if (mbAudioAbandonning) {
				if (++mAudioAbandonCount <= mAudioAbandonTotal) {
					bFlag = false;
				} else {
					LogAync(LOG_NOTICE, "RtpSession::SimPktLost( "
							"this : %p, "
							"[%s], [End], "
							"media_ssrc : 0x%08x(%u), "
							"total : %u "
							")", this, PktTypeDesc(media_ssrc).c_str(),
							media_ssrc, media_ssrc, mAudioAbandonTotal);

					mbAudioAbandonning = false;
					mAudioAbandonTotal = 0;
					mAudioAbandonCount = 0;
				}
			} else {
				int check = rand() % gAudioLostSeed;
				if (check == 0) {
					// 模拟丢包
					mbAudioAbandonning = true;
					mAudioAbandonTotal = rand() % gAudioLostSize + 1;
					mAudioAbandonCount = 0;
					bFlag = false;

					LogAync(LOG_NOTICE, "RtpSession::SimPktLost( "
							"this : %p, "
							"[%s], [Start], "
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
					LogAync(LOG_NOTICE, "RtpSession::SimPktLost( "
							"this : %p, "
							"[%s], [End], "
							"media_ssrc : 0x%08x(%u), "
							"total : %u "
							")", this, PktTypeDesc(media_ssrc).c_str(),
							media_ssrc, media_ssrc, mVideoAbandonTotal);

					mbVideoAbandonning = false;
					mVideoAbandonTotal = 0;
					mVideoAbandonCount = 0;
				}
			} else {
				int check = rand() % gVideoLostSeed;
				if (check == 0) {
					// 模拟丢包
					mbVideoAbandonning = true;
					mVideoAbandonTotal = rand() % gVideoLostSize + 1;
					mVideoAbandonCount = 0;
					bFlag = false;

					LogAync(LOG_NOTICE, "RtpSession::SimPktLost( "
							"this : %p, "
							"[%s], [Start], "
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

void RtpSession::OnReceiveBitrateChanged(const std::vector<uint32_t>& ssrcs,
		uint32_t bitrate) {
	for(std::vector<uint32_t>::const_iterator itr = ssrcs.begin(); itr != ssrcs.end(); itr++) {
		if ( IsVideoPkt(*itr) ) {
		    StreamStatistician* ss = rs_module_.GetStatistician(*itr);
			if ( ss ) {
				RtpReceiveStats rrs = ss->GetStats();
				LogAync(LOG_DEBUG, "RtpSession::OnReceiveBitrateChanged( "
						"this : %p, "
						"[%s], "
						"media_ssrc : 0x%08x(%u), "
						"bitrate_est : %u, "
						"bitrate_recv : %u, "
						"packets_lost : %d, "
						"jitter : %u, "
						"fraction : %d%% "
//						"jitter_since_last : %u, "
//						"packets_lost_since_last : %d, "
//						"fraction_since_last : %hhu "
						")",
						this,
						PktTypeDesc(*itr).c_str(), *itr, *itr,
						bitrate,
						ss->BitrateReceived(),
						rrs.packets_lost,
						rrs.jitter,
						ss->GetFractionLostInPercent()?*ss->GetFractionLostInPercent():0
//						index > -1?result[index].jitter_:0,
//						index > -1?result[index].cumulative_lost_:0,
//						index > -1?result[index].fraction_lost_:0
						);
			}

			if ( gAutoBitrate ) {
				const int64_t kSendThresholdPercent = 97;
				int64_t receive_bitrate_bps = static_cast<int64_t>(bitrate);
				int64_t now_ms = TimeMillis();
				// If we already have an estimate, check if the new total estimate is below
				// kSendThresholdPercent of the previous estimate.
				if (last_send_bitrate_bps_ > 0) {
					int64_t new_remb_bitrate_bps =
							last_send_bitrate_bps_ - bitrate_bps_ + receive_bitrate_bps;

					if (new_remb_bitrate_bps <
							kSendThresholdPercent * last_send_bitrate_bps_ / 100) {
						// The new bitrate estimate is less than kSendThresholdPercent % of the
						// last report. Send a REMB asap.
						last_remb_time_ms_ = now_ms - kRembSendIntervalMs;
					}
				}
				bitrate_bps_ = receive_bitrate_bps;

				if (now_ms - last_remb_time_ms_ < kRembSendIntervalMs) {
					break;
				}

				// NOTE: Updated if we intend to send the data; we might not have
				// a module to actually send it.
				last_remb_time_ms_ = now_ms;
				last_send_bitrate_bps_ = receive_bitrate_bps;
				// Cap the value to send in remb with configured value.
				receive_bitrate_bps = std::min(receive_bitrate_bps, max_bitrate_bps_);

				// 发送控制包
				SendRtcpRemb(*itr, receive_bitrate_bps);
			}
			break;
		}
	}
}
} /* namespace mediaserver */
