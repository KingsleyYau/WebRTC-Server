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
#include <rtp/RtpClient.h>
#include <socket/ISocketSender.h>

namespace mediaserver {

class WebRTC : public SocketSender, IceClientCallback {
public:
	WebRTC();
	virtual ~WebRTC();

	bool Init();
	void SetRemoteSdp(const string& sdp);

private:
	// SocketSender Implement
	int SendData(const void *data, unsigned int len);
	// IceClientCallback Implement
	void OnIceHandshakeFinish(IceClient *ice);
	void OnIceRecvData(IceClient *ice, const char *data, unsigned int size);

private:
	IceClient mIceClient;
	RtpClient mRtpClient;
};

} /* namespace mediaserver */

#endif /* WEBRTC_WEBRTC_H_ */
