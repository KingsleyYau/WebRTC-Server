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
}

RtpRawClient::~RtpRawClient() {
	// TODO Auto-generated destructor stub
}

bool RtpRawClient::Init(const string& sendIp, int rtpSendPort, int rtpRecvPort) {
	bool bFlag = true;

	bFlag &= mRtpSender.Init(sendIp, rtpSendPort, rtpRecvPort);
	bFlag &= mRtcpSender.Init(sendIp, rtpSendPort + 1, rtpRecvPort);

	if( bFlag ) {
		LogAync(
				LOG_WARNING,
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

	return bFlag;
}

} /* namespace mediaserver */
