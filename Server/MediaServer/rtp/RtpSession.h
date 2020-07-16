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

#include <rtp/NtpTime.h>

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

namespace mediaserver {
typedef KSafeMap<int, int> LostPacketMap;

class RtpTccFbItem {
public:
	unsigned short seq;
	unsigned long long recv_timestamp_us;
};

class RtpSession {
public:
	RtpSession();
	virtual ~RtpSession();

public:
	static bool GobalInit();
	static bool IsRtp(const char *frame, unsigned len);
	static bool IsRtcp(const char *frame, unsigned len);
	static unsigned int GetRtpSSRC(void *pkt, unsigned int& pktSize);
	static unsigned int GetRtcpSSRC(void *pkt, unsigned int& pktSize);

public:
	void SetRtpSender(SocketSender *sender);
	void SetRtcpSender(SocketSender *sender);
	void SetVideoSSRC(unsigned int ssrc);
	void SetAudioSSRC(unsigned int ssrc);

public:
	bool Start(char *localKey = NULL, int localSize = 0, char *remoteKey = NULL, int remoteSize = 0);
	void Stop();
	/**
	 * 设置模拟丢包
	 * @param bSimLost 是否启动模拟丢包
	 * @param audioLostSeed 音频丢包随机数, 满足 ( rand() % 随机数 == 0) 则开始丢包
	 * @param audioLostSize 音频开始连续丢包的个数
	 * @param videoLostSeed 视频丢包随机数, 满足 ( rand() % 随机数 == 0) 则开始丢包
	 * @param videoLostSize 视频开始连续丢包的个数
	 */
	 void SetSimLostParam(
			 bool bSimLost,
			 unsigned int audioLostSeed,
			 unsigned int audioLostSize,
			 unsigned int videoLostSeed,
			 unsigned int videoLostSize
			 );

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
	bool RecvRtpPacket(const char* frame, unsigned int size, void *pkt, unsigned int& pktSize);
	/**
	 * 发送原始RTP包(网络字节序)
	 * @param pkt 原始RTP数据包
	 * @param pktSize 原始RTP数据包大小
	 */
	bool SendRtpPacket(void *pkt, unsigned int& pktSize);
	/**
	 * 接收原始RTCP包(网络字节序)
	 * @param frame SRTCP数据包
	 * @param size SRTCP数据包大小
	 * @param pkt RTCP数据包
	 * @param pktSize RTCP数据包大小
	 */
	bool RecvRtcpPacket(const char* frame, unsigned int size, void *pkt, unsigned int& pktSize);
	/**
	 * 发送原始RTCP包(网络字节序)
	 * @param pkt 原始RTCP数据包
	 * @param pktSize 原始RTCP数据包大小
	 */
	bool SendRtcpPacket(void *pkt, unsigned int& pktSize);

public:
	/**
	 * Send Picture Loss Indication(PLI)
	 * 仅用于丢包, 不会携带视频信息(如H264的SPS和PPS)
	 * @param mediaSSRC 媒体流SSRC
	 */
	bool SendRtcpPLI(unsigned int mediaSSRC);
	/**
	 * Send Full Intra Request (FIR)
	 * 强制刷新视频信息(如H264的SPS和PPS)
	 * @param mediaSSRC 媒体流SSRC
	 */
	bool SendRtcpFIR(unsigned int mediaSSRC);
	/**
	 * 发送丢包重传请求
	 * @param mediaSSRC 媒体流SSRC
	 * @param start 重传包起始seq
	 * @param size 重传包长度大小
	 */
	bool SendRtcpNack(unsigned int mediaSSRC, unsigned int start, unsigned int size);
	/**
	 * Send Receiver Estimated Max Bitrate (REMB)
	 * 发送接收端码率控制包
	 * @param mediaSSRC 媒体流SSRC
	 * @param bitrate 需要控制的码率
	 */
	bool SendRtcpRemb(unsigned int mediaSSRC, unsigned long long bitrate = 2000000);
	/**
	 * Send Receiver Transport Feedback (TCC-FB)
	 * 发送接收端反馈
	 * @param mediaSSRC 媒体流SSRC
	 */
	bool SendRtcpTccFb(unsigned int mediaSSRC);

private:
	/**
	 * 重置
	 */
	void Reset();
	/**
	 * Send RTCP Receiver Report
	 * 发送接收者报告
	 */
	bool SendRtcpRR();
	/**
	 * 更新媒体信息(时间戳/帧号/丢包信息)
	 * @param pkt 原始RTP数据包
	 * @param pktSize 原始RTP数据包大小
	 */
	void UpdateStreamInfo(const void *pkt, unsigned int pktSize);
	/**
	 * 更新丢包统计
	 * @param ssrc 媒体流SSRC
	 * @param seq 媒体流当前数据包帧号
	 * @param lastMaxSeq 媒体流数据包最大帧号
	 * @param ts 媒体流当前数据包时间戳
	 * @param lastMaxTs 媒体流数据包最大时间戳
	 */
	bool UpdateLossPacket(unsigned int ssrc, unsigned int seq, unsigned int lastMaxSeq, unsigned int ts, unsigned int lastMaxTs);
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
	/**
	 * 根据媒体流包SSRC获取描述
	 * @param ssrc 媒体流SSRC
	 * @return 媒体流描述字符串
	 */
	string PktTypeDesc(unsigned int ssrc);

protected:
	// Status
	KMutex mClientMutex;
	bool mRunning;

private:
	// Socket
	SocketSender *mpRtpSender;
	SocketSender *mpRtcpSender;

	/////////////////////////////////////////////////////////////////////////////
	// Video
	// Max Video Timestamp
	unsigned int mVideoMaxTimestamp;
	// PLI Video Timestamp
	unsigned int mVideoPLITimestamp;
	// Max Video Sequence
	uint16_t mVideoMaxSeq;
	uint16_t mVideoMaxSeqLast;
	// Video Frame Count
	uint16_t mVideoFrameCount;
	// Video SSRC
	unsigned int mVideoSSRC;
	// Lost Video Packet Map
	LostPacketMap mVideoLostPacketMap;

	// Total Receive Video Packet
	unsigned int mVideoTotalRecvPacket;
	unsigned int mVideoTotalRecvPacketLast;
	// Last Max Video Sequence
	unsigned int mVideoLastMaxSeq;
	// Last Total Receive Video Packet
	unsigned int mVideoLastTotalRecvPacket;

	// Video LSR & DLSR
	uint32_t mVideoLSR;
	uint32_t mVideoDLSR;
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// Audio
	// Max Audio timestamp
	unsigned int mAudioMaxTimestamp;
	// Max Audio sequence
	uint16_t mAudioMaxSeq;
	uint16_t mAudioMaxSeqLast;
	// Audio SSRC
	unsigned int mAudioSSRC;
	// Lost Audio Packet map
	LostPacketMap mAudioLostPacketMap;

	// Total Receive Audio Packet
	unsigned int mAudioTotalRecvPacket;
	unsigned int mAudioTotalRecvPacketLast;
	// Last Max Audio Sequence
	unsigned int mAudioLastMaxSeq;
	// Last Total Receive Audio Packet
	unsigned int mAudioLastTotalRecvPacket;

	// Audio LSR & DLSR
	uint32_t mAudioLSR;
	uint32_t mAudioDLSR;
	/////////////////////////////////////////////////////////////////////////////

	// libsrtp
	srtp_ctx_t *mpSendSrtpCtx;
	srtp_policy_t *mpSendPolicy;
	srtp_ctx_t *mpRecvSrtpCtx;
	srtp_policy_t *mpRecvPolicy;

    // 请求强制刷新关键帧的序号
    unsigned int mFirSeq;
    // 收到的完整视频帧数量
    unsigned int mVideoRecvFrameCount;

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
};

} /* namespace mediaserver */

#endif /* RTP_RTPSESSION_H_ */
