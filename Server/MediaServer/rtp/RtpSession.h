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

public:
	bool StartSend(char *localKey, int size);
	void StopSend();
	bool StartRecv(char *remoteKey, int size);
	void StopRecv();

public:
	/**
	 * 发送原始RTP包(网络字节序)
	 */
	bool SendRtpPacket(void *pkt, unsigned int& pktSize);
	/**
	 * 接收原始RTP包(网络字节序)
	 */
	bool RecvRtpPacket(const char* frame, unsigned int size, void *pkt, unsigned int& pktSize);

	/**
	 * 发送原始RTCP包(网络字节序)
	 */
	bool SendRtcpPacket(void *pkt, unsigned int& pktSize);
	/**
	 * 接收原始RTCP包(网络字节序)
	 */
	bool RecvRtcpPacket(const char* frame, unsigned int size, void *pkt, unsigned int& pktSize);

public:
	/**
	 * Send Picture Loss Indication(PLI)
	 * 仅用于丢包, 不会携带视频信息(如H264的SPS和PPS)
	 *
	 */
	bool SendRtcpPLI(unsigned int mediaSSRC);
	/**
	 * Send Full Intra Request (FIR)
	 * 强制刷新视频信息(如H264的SPS和PPS)
	 */
	bool SendRtcpFIR(unsigned int mediaSSRC);

private:
	/**
	 * 重置
	 */
	void Reset();
	/**
	 * 发送丢包重传请求
	 */
	bool SendRtcpNack(unsigned int mediaSSRC, void* nacks, int size);
	/**
	 * Send RTCP Receiver Report
	 * 发送接收者报告
	 */
	bool SendRtcpRR(unsigned int mediaSSRC, uint32_t lsr, uint32_t dlsr);
	/**
	 * 更新媒体信息(时间戳/帧号/丢包信息)
	 */
	void UpdateStreamInfo(const void *pkt, unsigned int pktSize);
	/**
	 * 更新丢包统计
	 */
	bool UpdateLossPacket(unsigned int ssrc, unsigned int seq, unsigned int lastMaxSeq, unsigned int ts, unsigned int lastMaxTs);
	/**
	 * 更新媒体信息(RTT)
	 */
	void UpdateStreamInfoWithRtcp(const void *pkt, unsigned int pktSize);

protected:
	// Status
	KMutex mClientMutex;
	bool mRunning;

private:
	// Socket
	SocketSender *mpRtpSender;
	SocketSender *mpRtcpSender;

	// Video
	// Max Video Timestamp
	unsigned int mVideoMaxTimestamp;
	// Max Video Sequence
	uint16_t mVideoMaxSeq;
	// Video Frame Count
	uint16_t mVideoFrameCount;
	// Video SSRC
	unsigned int mVideoSSRC;
	// Lost Video Packet Map
	LostPacketMap mVideoLostPacketMap;

	// Total Receive Video Packet
	unsigned int mVideoTotalRecvPacket;
	// Last Max Video Sequence
	unsigned int mVideoLastMaxSeq;
	// Last Total Receive Video Packet
	unsigned int mVideoLastTotalRecvPacket;

	// Audio
	// Max Audio timestamp
	unsigned int mAudioMaxTimestamp;
	// Max Audio sequence
	uint16_t mAudioMaxSeq;
	// Audio SSRC
	unsigned int mAudioSSRC;
	// Lost Audio Packet map
	LostPacketMap mAudioLostPacketMap;

	// Total Receive Audio Packet
	unsigned int mAudioTotalRecvPacket;
	// Last Max Audio Sequence
	unsigned int mAudioLastMaxSeq;
	// Last Total Receive Audio Packet
	unsigned int mAudioLastTotalRecvPacket;

	// Last Rtcp Sender Report Timestamp
	NtpTime mRtcpLSR;

	// libsrtp
	srtp_ctx_t *mpSendSrtpCtx;
	srtp_policy_t *mpSendPolicy;
	srtp_ctx_t *mpRecvSrtpCtx;
	srtp_policy_t *mpRecvPolicy;

    // 请求强制刷新关键帧的序号
    unsigned int mFirSeq;
    // 收到的完整视频帧数量
    unsigned int mVideoRecvFrameCount;
};

} /* namespace mediaserver */

#endif /* RTP_RTPSESSION_H_ */
