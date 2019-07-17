/*
 * RtpClient.h
 *
 *  Created on: 2019/06/20
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_RTPCLIENT_H_
#define RTP_RTPCLIENT_H_

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
typedef struct srtp_policy_t;

namespace mediaserver {

/*
 * RTP_HEADER_LEN indicates the size of an RTP header
 */
#define MTU 1500
#define RTP_HEADER_LEN 12
#define RTP_HEADER_SIZE 54
#define RTP_MAX_BUF_LEN (MTU - RTP_HEADER_SIZE)

typedef struct {
    unsigned char version : 2; /* protocol version       */
    unsigned char p : 1;       /* padding flag           */
    unsigned char x : 1;       /* header extension flag  */
    unsigned char cc : 4;      /* CSRC count             */
    unsigned char m : 1;       /* marker bit             */
    unsigned char pt : 7;      /* payload type           */
    uint16_t seq;              /* sequence number        */
    uint32_t ts;               /* timestamp              */
    uint32_t ssrc;             /* synchronization source */
} RtpHeader;

typedef struct {
	RtpHeader header;
    char body[RTP_MAX_BUF_LEN];
} RtpPacket;

class RtpClient {
public:
	RtpClient();
	virtual ~RtpClient();

public:
	static bool GobalInit();
	static bool IsRtp(const char *frame, unsigned len);

public:
	void SetSocketSender(SocketSender *sender);

public:
	bool Start();
	void Stop();

public:
	bool StartRecv(char *remoteKey, int size);
	void StopRecv();

public:
	void SetVideoKeyFrameInfoH264(const char *sps, int spsSize, const char *pps, int ppsSize, int naluHeaderSize, u_int32_t timestamp);
	bool SendVideoFrameH264(const char* frame, unsigned int size, unsigned int timestamp);

	bool SendAudioFrame(const char* frame, unsigned int size, unsigned int timestamp);

	bool SendRtpPacket(RtpPacket *pkt, unsigned int& pktSize);
	bool RecvRtpPacket(const char* frame, unsigned int size, RtpPacket *pkt, unsigned int& pktSize);

private:
	void Reset();
	bool SendVideoKeyFrameH264();

private:
	// Status
	KMutex mClientMutex;
	bool mRunning;

	// Socket
	SocketSender *mpSocketSender;

	// Video
	// Original video timestamp
	unsigned int mVideoTimestamp;
	// RTP session video timestamp
	unsigned int mVideoRtpTimestamp;
	// RTP session video sequence
	uint16_t mVideoRtpSeq;
	// Video frame count
	uint16_t mVideoFrameCount;

	// Audio
	// Original audio timestamp
	unsigned int mAudioTimestamp;
	// RTP session audio timestamp
	unsigned int mAudioRtpTimestamp;
	// RTP session audio sequence
	uint16_t mAudioRtpSeq;

	// libsrtp
	srtp_ctx_t *mpSrtpCtx;
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

    // 是否正在接收数据
    bool mRecving;
};

} /* namespace mediaserver */

#endif /* RTP_RTPCLIENT_H_ */
