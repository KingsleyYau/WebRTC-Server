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
}

RtpRawClient::~RtpRawClient() {
	// TODO Auto-generated destructor stub
}

bool RtpRawClient::Init(const string& sendIp, int rtpSendPort, int rtpRecvPort) {
	bool bFlag = true;

	mClientMutex.lock();
	if( !mRunning ) {
		mSendIp = sendIp;
		mRtpSendPort = rtpSendPort;

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
					LOG_ALERT,
					"RtpRawClient::Init( "
					"this : %p, "
					"[Fail], "
					"sendIp : %s, "
					"rtpSendPort : %d, "
					"rtpRecvPort : %d "
					")",
					this,
					sendIp.c_str(),
					rtpSendPort,
					rtpRecvPort
					);
		}
	}
	mClientMutex.unlock();

	return bFlag;
}

bool RtpRawClient::Start(char *localKey, int localSize, char *remoteKey, int remoteSize) {
	bool bFlag = RtpSession::Start(localKey, localSize, remoteKey, remoteSize);

	bFlag &= mRtpSender.Init(mSendIp, mRtpSendPort);
	bFlag &= mRtcpSender.Init(mSendIp, mRtpSendPort + 1);

	return bFlag;
}

void RtpRawClient::Stop() {
	RtpSession::Stop();
	mRtpSender.Close();
	mRtcpSender.Close();
}

} /* namespace mediaserver */
