/*
 * RtpRawClient.cpp
 *
 *  Created on: 2019/07/03
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "RtpRawClient.h"

// Common
#include <common/LogManager.h>

namespace mediaserver {

RtpRawClient::RtpRawClient() {
	// TODO Auto-generated constructor stub
	SetRtpSender(&mRtpSender);
	SetRtcpSender(&mRtcpSender);

	mSendIp = "";
	mRtpSendPort = 0;
	mRtpRecvPort = 0;
}

RtpRawClient::~RtpRawClient() {
	// TODO Auto-generated destructor stub
}

bool RtpRawClient::Init(const string& sendIp, int rtpSendPort, const string& recvIp, int rtpRecvPort) {
	bool bFlag = true;

	mClientMutex.lock();
	if( !mRunning ) {
		mSendIp = sendIp;
		mRtpSendPort = rtpSendPort;
		mRecvIp = recvIp;
		mRtpRecvPort = rtpRecvPort;

		if( bFlag ) {
			LogAync(
					LOG_DEBUG,
					"RtpRawClient::Init( "
					"this : %p, "
					"[OK], "
					"sendIp : %s, "
					"rtpSendPort : %d, "
					"rtpRecvPort : %d "
					")",
					this,
					sendIp.c_str(),
					rtpSendPort,
					rtpRecvPort
					);
		} else {
			LogAync(
					LOG_WARNING,
					"RtpRawClient::Init( "
					"this : %p, "
					"[Fail], "
					"sendIp : %s, "
					"rtpSendPort : %d, "
					"recvIp : %s, "
					"rtpRecvPort : %d "
					")",
					this,
					sendIp.c_str(),
					rtpSendPort,
					recvIp.c_str(),
					rtpRecvPort
					);
		}
	}
	mClientMutex.unlock();

	return bFlag;
}

bool RtpRawClient::Start(char *localKey, int localSize, char *remoteKey, int remoteSize) {
	bool bFlag = RtpSession::Start(localKey, localSize, remoteKey, remoteSize);

	if ( mRtpSendPort != -1 ) {
		bFlag &= mRtpSender.Init(mSendIp, mRtpSendPort);
		bFlag &= mRtcpSender.Init(mSendIp, mRtpSendPort + 1);
	}
	if ( mRtpRecvPort != -1 ) {
		bFlag &= mRtpReceiver.Init(mRecvIp, mRtpRecvPort);
	}

	return bFlag;
}

void RtpRawClient::Stop() {
	RtpSession::Stop();
	mRtpSender.Close();
	mRtcpSender.Close();
	mRtpReceiver.Close();
}

void RtpRawClient::Shutdown() {
	LogAync(
			LOG_INFO,
			"RtpRawClient::Shutdown( "
			"this : %p "
			")",
			this
			);
	mRtpReceiver.Shutdown();
}

bool RtpRawClient::RecvRtpPacket(void *pkt, unsigned int& pktSize) {
	bool bFlag = false;
	char buffer[2048] = {'\0'};
	int size = 	mRtpReceiver.RecvData(buffer, sizeof(buffer));
	if ( size > 0 ) {
		RtpSession::RecvRtpPacket(buffer, size, pkt, pktSize);
		bFlag = true;
	} else if ( size == 0 ) {
		bFlag = true;
	} else {
		LogAync(
				LOG_INFO,
				"RtpRawClient::RecvRtpPacket( "
				"this : %p, "
				"[Fail], "
				"size : %d "
				")",
				this,
				size
				);
	}
	return bFlag;
}

} /* namespace mediaserver */
