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

#include <media/VideoMuxer.h>

#include <socket/ISocketSender.h>

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
	 * 设置H264视频信息
	 */
	void SetVideoKeyFrameInfoH264(const char *sps, int spsSize, const char *pps, int ppsSize, int naluHeaderSize, u_int32_t timestamp);
	/**
	 * 发送一个完整的H264帧
	 */
	bool SendVideoFrameH264(const char* frame, unsigned int size, unsigned int timestamp);

	bool SendAudioFrame(const char* frame, unsigned int size, unsigned int timestamp);

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
	bool SendRtcpPLI(unsigned int remoteSSRC);
	/**
	 * Send Full Intra Request (FIR)
	 * 强制刷新视频信息(如H264的SPS和PPS)
	 */
	bool SendRtcpFIR(unsigned int remoteSSRC);

private:
	void Reset();
	bool SendVideoKeyFrameH264();

protected:
	// Status
	KMutex mClientMutex;
	bool mRunning;

private:
	// Socket
	SocketSender *mpRtpSender;
	SocketSender *mpRtcpSender;

	// Video
	// Original video timestamp
	unsigned int mVideoTimestamp;
	// RTP session video timestamp
	unsigned int mVideoRtpTimestamp;
	// RTP session video sequence
	uint16_t mVideoRtpSeq;
	// Video frame count
	uint16_t mVideoFrameCount;
	// Video SSRC
	unsigned int mVideoSSRC;

	// Audio
	// Original audio timestamp
	unsigned int mAudioTimestamp;
	// RTP session audio timestamp
	unsigned int mAudioRtpTimestamp;
	// RTP session audio sequence
	uint16_t mAudioRtpSeq;
	// Audio SSRC
	unsigned int mAudioSSRC;

	// libsrtp
	srtp_ctx_t *mpSendSrtpCtx;
	srtp_policy_t *mpSendPolicy;
	srtp_ctx_t *mpRecvSrtpCtx;
	srtp_policy_t *mpRecvPolicy;

    // H264格式转换器
    VideoMuxer mVideoMuxer;
	int mNaluHeaderSize;

    // 发包参数
    u_int32_t mInputVideoTimestamp;
    u_int32_t mSendVideoFrameTimestamp;

    // 收包参数
    char *mpSps;
    int mSpsSize;
    char *mpPps;
    int mPpsSize;

    // 请求强制刷新关键帧的序号
    unsigned int mFirSeq;
    // 收到的完整视频帧数量
    unsigned int mVideoRecvFrameCount;
};

} /* namespace mediaserver */

#endif /* RTP_RTPSESSION_H_ */
