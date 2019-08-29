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

bool RtpRawClient::Init(const string sendIp, int rtpSendPort, int rtpRecvPort) {
	bool bFlag = true;

	mClientMutex.lock();
	if( !mRunning ) {
		mSendIp = sendIp;
		mRtpSendPort = rtpSendPort;
		mRtpReceiver.Init(sendIp, rtpRecvPort);

		if( bFlag ) {
			LogAync(
					LOG_STAT,
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
					LOG_ERR_SYS,
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
	mRtpReceiver.Close();
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
	}
	return bFlag;
}

} /* namespace mediaserver */
