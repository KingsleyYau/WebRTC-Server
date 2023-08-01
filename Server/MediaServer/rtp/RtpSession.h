/*
 * RtpSession.h
 * SRTP会话管理器
 *  Created on: 2019/06/20
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_RTPSESSION_H_
#define RTP_RTPSESSION_H_

#include <common/KMutex.h>
#include <common/KSafeMap.h>
#include <common/KSafeList.h>

#include <media/VideoMuxer.h>

#include <socket/ISocketSender.h>

#include <rtp/base/ntp_time.h>
#include <rtp/packet/SenderReport.h>
#include <rtp/packet/rtp_packet_received.h>
#include <rtp/packet/Dlrr.h>

#include <rtp/modules/nack_module.h>
#include <rtp/modules/nack_audio_module.h>
#include <rtp/modules/remote_bitrate_estimator/remote_bitrate_estimator_abs_send_time.h>
#include <rtp/include/receive_statistics_impl.h>
#include <rtp/include/rtp_packet_history.h>
#include <rtp/packet/rtp_packet_received.h>
using namespace qpidnetwork::rtcp;

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>
using namespace std;

typedef struct srtp_ctx_t_ srtp_ctx_t;
struct srtp_policy_t;

/*
 * RTP_HEADER_LEN indicates the size of an RTP header
 */
#define MTU 1536
#define UDP_HEADER_LEN 42
#define RTP_HEADER_LEN 12
#define RTP_MAX_PAYLOAD_LEN (MTU - UDP_HEADER_LEN - RTP_HEADER_LEN)
#define RTP_MAX_LEN (MTU - UDP_HEADER_LEN)

namespace qpidnetwork {
class RtpPacket;
class RtpSession:public RemoteBitrateObserver {
public:
	RtpSession();
	virtual ~RtpSession();

public:
	static bool GobalInit();
	static void SetGobalParam(unsigned int maxPliSeconds, bool autoBitrate = true, unsigned int videoMinBitrate = 200000, unsigned int videoMaxBitrate = 1000 * 1000, bool simLost = false);
	static bool IsRtp(const char *frame, unsigned len);
	static bool IsRtcp(const char *frame, unsigned len);
	static unsigned int GetRtpSSRC(void *pkt, unsigned int& pktSize);
	static unsigned int GetRtcpSSRC(void *pkt, unsigned int& pktSize);

public:
	void SetRtpSender(SocketSender *sender);
	void SetRtcpSender(SocketSender *sender);
	void SetVideoSSRC(unsigned int ssrc);
	void SetAudioSSRC(unsigned int ssrc);
	void SetIdentification(string identification);

	void RegisterVideoExtensions(const vector<RtpExtension>& extensions);
	void RegisterAudioExtensions(const vector<RtpExtension>& extensions);

public:
	bool Start(char *localKey = NULL, int localSize = 0, char *remoteKey = NULL,
			int remoteSize = 0);
	void Stop();
	/**
	 * 设置模拟丢包
	 * @param bSimLost 是否启动模拟丢包
	 * @param audioLostSeed 音频丢包随机数, 满足 ( rand() % 随机数 == 0) 则开始丢包
	 * @param audioLostSize 音频开始连续丢包的个数
	 * @param videoLostSeed 视频丢包随机数, 满足 ( rand() % 随机数 == 0) 则开始丢包
	 * @param videoLostSize 视频开始连续丢包的个数
	 */
	void SetSimLostParam(bool bSimLost, unsigned int audioLostSeed,
			unsigned int audioLostSize, unsigned int videoLostSeed,
			unsigned int videoLostSize);

public:
	bool StartSend(char *localKey, int size);
	void StopSend();
	bool StartRecv(char *remoteKey, int size);
	void StopRecv();

public:
	/**
	 * 接收原始RTP包(网络字节序)
	 * @param frame SRTP数据包
	 * @param size SRTP数据包大小
	 * @param pkt RTP数据包
	 * @param pktSize RTP数据包大小
	 */
	bool RecvRtpPacket(const char* frame, unsigned int size, void *pkt,
			unsigned int& pktSize);
	/**
	 * 发送原始RTP包(网络字节序)
	 * @param pkt 原始RTP数据包
	 * @param pktSize 原始RTP数据包大小
	 */
	bool SendRtpPacket(void *pkt, unsigned int& pktSize);
	bool EnqueueRtpPacket(void *pkt, unsigned int& pktSize);
	/**
	 * 接收原始RTCP包(网络字节序)
	 * @param frame SRTCP数据包
	 * @param size SRTCP数据包大小
	 * @param pkt RTCP数据包
	 * @param pktSize RTCP数据包大小
	 */
	bool RecvRtcpPacket(const char* frame, unsigned int size, void *pkt,
			unsigned int& pktSize);
	/**
	 * 发送原始RTCP包(网络字节序)
	 * @param pkt 原始RTCP数据包
	 * @param pktSize 原始RTCP数据包大小
	 */
	bool SendRtcpPacket(void *pkt, unsigned int& pktSize);

	/**
	 * 是否已经接收过视频帧
	 */
	bool IsReceivedVideo();

private:
	/**
	 * Send Picture Loss Indication(PLI)
	 * 仅用于丢包, 不会携带视频信息(如H264的SPS和PPS)
	 * @param media_ssrc 媒体流SSRC
	 */
	bool SendRtcpPli(unsigned int media_ssrc);
	/**
	 * Send Full Intra Request (FIR)
	 * 强制刷新视频信息(如H264的SPS和PPS)
	 * @param media_ssrc 媒体流SSRC
	 */
	bool SendRtcpFir(unsigned int media_ssrc);
	/**
	 * 发送丢包重传请求
	 * @param media_ssrc 媒体流SSRC
	 * @param start 重传包起始seq
	 * @param size 重传包长度大小
	 */
	bool SendRtcpNack(unsigned int media_ssrc, unsigned int start,
			unsigned int size);
	bool SendRtcpNack(unsigned int media_ssrc, const std::vector<uint16_t> &nack_batch);

	/**
	 * Send Receiver Estimated Max Bitrate (REMB)
	 * 发送接收端码率控制包
	 * @param media_ssrc 媒体流SSRC
	 * @param bitrate 需要控制的码率
	 */
	bool SendRtcpRemb(unsigned int media_ssrc, unsigned long long bitrate =
			2000000);
	/**
	 * Send Receiver Transport Feedback (TCC-FB)
	 * 发送接收端反馈
	 * @param media_ssrc 媒体流SSRC
	 */
	bool SendRtcpTccFB(unsigned int media_ssrc);
	/**
	 * rtcp-xr方案
	 */
	bool SendRtcpXr();
	/**
	 * Send RTCP Receiver Report
	 * 发送接收者报告
	 */
	bool SendRtcpRr(const std::vector<rtcp::ReportBlock> &result);

private:
	void OnReceiveBitrateChanged(const std::vector<uint32_t>& ssrcs,
	                                       uint32_t bitrate);
private:
	/**
	 * 重置
	 */
	void Reset();
	/**
	 * 更新媒体信息(时间戳/帧号/丢包信息)
	 * @param rtpPkt 原始RTP数据包
	 * @param recvTime 原始RTP数据包到达时间
	 */
	void UpdateStreamInfo(const RtpPacketReceived *rtpPkt, uint64_t recvTime, const RTPHeader& header);
	/**
	 * 更新收包统计
	 */
	bool UpdateStatsPacket(const RtpPacketReceived *rtpPkt, uint64_t recvTime);
	/**
	 * 更新音频丢包统计, 音频不支持Nack
	 */
	bool UpdateAudioLossPacket(const RtpPacketReceived *rtpPkt, uint64_t recvTime);
	/**
	 * 处理视频丢包, 发送Nack
	 */
	bool UpdateVideoLossPacket(const RtpPacketReceived *rtpPkt, uint64_t recvTime);
	/**
	 * 更新媒体信息(RTT)
	 * @param pkt 原始RTCP数据包
	 * @param pktSize 原始RTCP数据包大小
	 */
	void UpdateStreamInfoWithRtcp(const void *pkt, unsigned int pktSize);
	/**
	 * 模拟丢包(测试用)
	 * @param ssrc 媒体流SSRC
	 * @param seq 媒体流当前数据包帧号
	 */
	bool SimPktLost(unsigned int ssrc, unsigned int seq);
	/**
	 * 判断是否音频数据包
	 * @param ssrc 媒体流SSRC
	 * @return TRUE/FALSE
	 */
	bool IsAudioPkt(unsigned int ssrc);
	bool IsVideoPkt(unsigned int ssrc);
	/**
	 * 根据媒体流包SSRC获取描述
	 * @param ssrc 媒体流SSRC
	 * @return 媒体流描述字符串
	 */
	string PktTypeDesc(unsigned int ssrc);

	void HandleRtcpSr(const CommonHeader& rtcp_block);
	void HandleRtcpRr(const CommonHeader& rtcp_block);
	void HandleRtcpRb(const ReportBlock& report_block);
	void HandleRtcpSdes(const CommonHeader& rtcp_block);
	void HandleRtcpXr(const CommonHeader& rtcp_block);
	void HandleRtcpXrDlrr(const ReceiveTimeInfo& rti);
	void HandleNack(const CommonHeader& rtcp_block);

protected:
	// Status
	KMutex mClientMutex;
	bool mRunning;

private:
	// Socket
	SocketSender *mpRtpSender;
	SocketSender *mpRtcpSender;

	// 唯一标识
	string mIdentification;

	/////////////////////////////////////////////////////////////////////////////
	// Video
	// PLI Video Timestamp
	unsigned int mVideoPLITimestamp;
	// Video SSRC
	unsigned int mVideoSSRC;
	// 请求强制刷新关键帧的序号
	unsigned int mFirSeq;
	// 收到的完整视频帧数量
	unsigned int mVideoRecvFrameCount;
	// 是否已经接收过视频帧
	bool mIsVideoReceived;
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// Audio
	// Audio SSRC
	unsigned int mAudioSSRC;
	// 是否已经接收过音频帧
	bool mIsAudioReceived;
	/////////////////////////////////////////////////////////////////////////////

	// libsrtp
	srtp_ctx_t *mpSendSrtpCtx;
	srtp_policy_t *mpSendPolicy;
	srtp_ctx_t *mpRecvSrtpCtx;
	srtp_policy_t *mpRecvPolicy;
	//////////////////////////////////////////////////////////////////////////
	/**
	 * 模拟丢包
	 */
	bool mbSimLost;
	unsigned int mAudioLostSeed;
	unsigned int mAudioLostSize;
	unsigned int mVideoLostSeed;
	unsigned int mVideoLostSize;
	// 模拟丢包状态
	bool mbAudioAbandonning;
	// 连续丢包数量
	int mAudioAbandonTotal;
	// 当前已丢包数量
	int mAudioAbandonCount;

	// 模拟丢包状态
	bool mbVideoAbandonning;
	// 连续丢包数量
	int mVideoAbandonTotal;
	// 当前已丢包数量
	int mVideoAbandonCount;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	RtpHeaderExtensionMap mVideoExtensionMap;
	RtpHeaderExtensionMap mAudioExtensionMap;
	//////////////////////////////////////////////////////////////////////////

	// RTCP统计数据
	int64_t xr_last_send_ms_;
	uint32_t remote_sender_rtp_time_;
	NtpTime remote_sender_ntp_time_;
	NtpTime last_received_sr_ntp_;
	int64_t xr_rr_rtt_ms_;
	int64_t last_rr_send_time_;
	int64_t last_stats_time_;

	int64_t last_remb_time_ms_;
	int64_t last_send_bitrate_bps_;
	int64_t bitrate_bps_;
	int64_t max_bitrate_bps_;

	// 码率滤波和丢包重传模块
	NackModule nack_module_;
	NackAudioModule nack_audio_module_;
	RemoteBitrateEstimatorAbsSendTime rbe_module_;
	ReceiveStatisticsImpl rs_module_;

	// RTP发包缓存模块
	RtpPacketHistory video_packet_history_;
	RtpPacketHistory audio_packet_history_;

	// 用于解析接收的rtp包
//	RtpPacketReceived rtpPktCache;
};

} /* namespace qpidnetwork */

#endif /* RTP_RTPSESSION_H_ */
