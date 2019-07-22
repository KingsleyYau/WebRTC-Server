/*
 * WebRTC.h
 *
 *  Created on: 2019/07/02
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef WEBRTC_WEBRTC_H_
#define WEBRTC_WEBRTC_H_

#include <ice/IceClient.h>
#include <rtp/DTLSClient.h>
#include <rtp/RtpClient.h>
#include <rtp/RtpRawClient.h>
#include <socket/ISocketSender.h>

namespace mediaserver {

class WebRTC : public SocketSender, IceClientCallback {
public:
	WebRTC();
	virtual ~WebRTC();

public:
	static bool GobalInit();

public:
	void SetRemoteSdp(const string& sdp);
	bool Start();
	void Stop();

private:
	// SocketSender Implement
	int SendData(const void *data, unsigned int len);
	// IceClientCallback Implement
	void OnIceHandshakeFinish(IceClient *ice);
	void OnIceRecvData(IceClient *ice, const char *data, unsigned int size, unsigned int streamId, unsigned int componentId);

private:
	IceClient mIceClient;
	DTLSClient mDTLSClient;
	RtpClient mRtpClient;

	RtpRawClient mRtpDstAudioClient;
	RtpRawClient mRtpDstVideoClient;

	unsigned int mVideoSSRC;
	unsigned int mAudioSSRC;
};

} /* namespace mediaserver */

#endif /* WEBRTC_WEBRTC_H_ */
