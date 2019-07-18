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
	mDTLSClient.SetSocketSender(this);
	mRtpClient.SetSocketSender(this);
}

WebRTC::~WebRTC() {
	// TODO Auto-generated destructor stub
}

bool WebRTC::GobalInit() {
	bool bFlag = true;

	bFlag &= IceClient::GobalInit();
	bFlag &= DTLSClient::GobalInit();
	bFlag &= RtpClient::GobalInit();

	LogAync(
			LOG_ERR_USER,
			"WebRTC::GobalInit( "
			"[%s] "
			")",
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

bool WebRTC::Start() {
	bool bFlag = true;

	bFlag &= mIceClient.Start();
	bFlag &= mDTLSClient.Start();

	mRtpRawClient.Init("192.168.88.138", 9999, 9999);
	mRtpRawClient.Start(NULL, 0, NULL, 0);

	LogAync(
			LOG_MSG,
			"WebRTC::Start( "
			"this : %p, "
			"[%s] "
			")",
			this,
			bFlag?"OK":"Fail"
			);

	if( !bFlag ) {
		Stop();
	}

	return bFlag;
}

void WebRTC::Stop() {
	mIceClient.Stop();
	mDTLSClient.Stop();
	mRtpClient.Stop();

	LogAync(
			LOG_MSG,
			"WebRTC::Stop( "
			"this : %p, "
			"[OK] "
			")",
			this
			);
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
			"ice : %p, "
			"local : %s, "
			"remote : %s "
			")",
			this,
			ice,
			ice->GetLocalAddress().c_str(),
			ice->GetRemoteAddress().c_str()
			);
	mDTLSClient.Handshake();
}

void WebRTC::OnIceRecvData(IceClient *ice, const char *data, unsigned int size, unsigned int streamId, unsigned int componentId) {
//	LogAync(
//			LOG_MSG,
//			"WebRTC::OnIceRecvData( "
//			"this : %p, "
//			"ice : %p, "
//			"streamId : %u, "
//			"componentId : %u, "
//			"size : %d, "
//			"data[0] : 0x%X "
//			")",
//			this,
//			ice,
//			streamId,
//			componentId,
//			size,
//			(unsigned char)data[0]
//			);

	bool bFlag = false;
	if( DTLSClient::IsDTLS(data, size) ) {
		bFlag = mDTLSClient.RecvFrame(data, size);
		if( bFlag ) {
			// Check Handshake status
			DTLSClientStatus status = mDTLSClient.GetClientStatus();
			if( status == DTLSClientStatus_HandshakeDone ) {
				LogAync(
						LOG_MSG,
						"WebRTC::OnIceRecvData( "
						"this : %p, "
						"[DTLS Handshake OK] "
						")",
						this
						);
				char localKey[SRTP_MASTER_LENGTH];
				int localSize = 0;
				mDTLSClient.GetClientKey(localKey, localSize);
				char remoteKey[SRTP_MASTER_LENGTH];
				int remoteSize = 0;
				mDTLSClient.GetServerKey(remoteKey, remoteSize);

				mRtpClient.Start(localKey, localSize, remoteKey, remoteSize);
			} else if ( status == DTLSClientStatus_Alert ) {
				LogAync(
						LOG_MSG,
						"WebRTC::OnIceRecvData( "
						"this : %p, "
						"[DTLS Alert] "
						")",
						this
						);
				mRtpClient.Stop();
			}
		}
	} else if( RtpClient::IsRtp(data, size) ) {
		RtpPacket pkt;
		unsigned int pktSize = size;
		bFlag = mRtpClient.RecvRtpPacket(data, size, &pkt, pktSize);
		if( bFlag ) {
			mRtpRawClient.SendRtpPacket(&pkt, pktSize);
		}

	} else {

	}

}

} /* namespace mediaserver */
