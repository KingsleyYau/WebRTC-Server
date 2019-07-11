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
	SetSocketSender(&mUdpClient);
}

RtpRawClient::~RtpRawClient() {
	// TODO Auto-generated destructor stub
}

bool RtpRawClient::Init(const string& sendIp, int sendPort, int recvPort) {
	bool bFlag = true;

	bFlag &= mUdpClient.Init(sendIp, sendPort, recvPort);

	if( bFlag ) {
		LogAync(
				LOG_WARNING,
				"RtpRawClient::Init( "
				"this : %p, "
				"[OK], "
				"sendIp : %s, "
				"sendPort : %d, "
				"recvPort : %d "
				")",
				this,
				sendIp.c_str(),
				sendPort,
				recvPort
				);
	} else {
		LogAync(
				LOG_ERR_SYS,
				"RtpRawClient::Init( "
				"this : %p, "
				"[Fail], "
				"sendIp : %s, "
				"sendPort : %d, "
				"recvPort : %d "
				")",
				this,
				sendIp.c_str(),
				sendPort,
				recvPort
				);
	}

	return bFlag;
}

void RtpRawClient::SetSocketSender(SocketSender *sender) {
	RtpClient::SetSocketSender(sender);
}

} /* namespace mediaserver */
