/*
 * WebRTC.cpp
 *
 *  Created on: 2019/07/02
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "WebRTC.h"

// Common
#include <common/LogManager.h>

namespace mediaserver {

WebRTC::WebRTC() {
	// TODO Auto-generated constructor stub
	mIceClient.SetCallback(this);
	mRtpClient.SetSocketSender(this);
}

WebRTC::~WebRTC() {
	// TODO Auto-generated destructor stub
}

bool WebRTC::Init() {
	bool bFlag = true;

	bFlag &= mIceClient.Init();
	bFlag &= mRtpClient.Init();

	LogAync(
			LOG_MSG,
			"WebRTC::Init( "
			"this : %p, "
			"[%s] "
			")",
			this,
			bFlag?"OK":"Fail"
			);

	return bFlag;
}

void WebRTC::SetRemoteSdp(const string& sdp) {
	LogAync(
			LOG_MSG,
			"WebRTC::SetRemoteSdp( "
			"this : %p, "
			"sdp : \n%s"
			")",
			this,
			sdp.c_str()
			);

	mIceClient.SetRemoteSdp(sdp);
}

int WebRTC::SendData(const void *data, unsigned int len) {
	// Send RTP data through ICE data channel
	return mIceClient.SendData(data, len);
}

void WebRTC::OnIceHandshakeFinish(IceClient *ice) {
	LogAync(
			LOG_MSG,
			"WebRTC::OnIceHandshakeFinish( "
			"this : %p, "
			"ice : %p "
			")",
			this,
			ice
			);
	mRtpClient.Handshake();
}

void WebRTC::OnIceRecvData(IceClient *ice, const char *data, unsigned int size) {
	LogAync(
			LOG_MSG,
			"WebRTC::OnIceRecvData( "
			"this : %p, "
			"ice : %p, "
			"size : %d, "
			"data[0] : 0x%02X "
			")",
			this,
			ice,
			size,
			data[0]
			);
	mRtpClient.RecvFrame(data, size);
}

} /* namespace mediaserver */
