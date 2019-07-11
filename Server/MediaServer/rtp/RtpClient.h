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

// libopenssl
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/asn1.h>
#include <openssl/bio.h>

#include <string>
using namespace std;

typedef struct srtp_ctx_t_ srtp_ctx_t;
//typedef struct ssl_st SSL;

namespace mediaserver {
class RtpClient {
public:
	RtpClient();
	virtual ~RtpClient();

public:
	static bool GobalInit();

public:
	bool Init();
	void SetSocketSender(SocketSender *sender);
	void Destroy();

	bool Handshake();
	void SetKeyFrameInfo(const char *sps, int spsSize, const char *pps, int ppsSize, int naluHeaderSize, u_int32_t timestamp);
	bool SendVideoFrame(const char* frame, unsigned int size, unsigned int timestamp);
	bool SendAudioFrame(const char* frame, unsigned int size, unsigned int timestamp);

	bool RecvFrame(const char* frame, unsigned int size);

private:
	static int Generate_SSL_Keys(X509** certificate, EVP_PKEY** privateKey);
	static bool Load_SSL_Keys(const char* server_pem, const char* server_key, X509** certificate, EVP_PKEY** privateKey);

private:
	void Reset();
	bool SendVideoKeyFrame();
	bool CheckHandshake();

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
	uint32_t mSSRC;

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

    // SSL
    SSL *mpSSL;
    // Read BIO (incoming DTLS data)
    BIO *mpReadBIO;
    // Write BIO (outgoing DTLS data)
    BIO *mpWriteBIO;
};

} /* namespace mediaserver */

#endif /* RTP_RTPCLIENT_H_ */
