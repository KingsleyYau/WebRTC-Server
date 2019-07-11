/*
 * UdpClient.cpp
 *
 *  Created on: 2019/07/02
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "UdpClient.h"

// Common
#include <common/LogManager.h>
#include <common/Math.h>
#include <common/CommonFunc.h>

namespace mediaserver {

UdpClient::UdpClient() {
	// TODO Auto-generated constructor stub
	mSendIp = "";
	mSendPort = -1;
	mRecvPort = -1;
	mFd = -1;
}

UdpClient::~UdpClient() {
	// TODO Auto-generated destructor stub
}

bool UdpClient::Init(const string& sendIp, int sendPort, int recvPort) {
	bool bFlag = true;

	mSendIp = sendIp;
	mSendPort = sendPort;
	mRecvPort = recvPort;

	// Check network address
	in_addr_t inAddrT = inet_addr(sendIp.c_str());
    if (0xFFFFFFFF == inAddrT) {
    	bFlag = false;
    }

    // Create socket
    if( bFlag ) {
    	mFd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    	if( mFd == -1 ) {
    		bFlag = false;
    	}
    }

    if( bFlag ) {
		struct in_addr inAddr = { s_addr:inAddrT };
		mSendSockAddr.sin_addr = inAddr;
		mSendSockAddr.sin_family = PF_INET;
		mSendSockAddr.sin_port = htons(mSendPort);
    }

	if( bFlag ) {
		LogAync(
				LOG_STAT,
				"UdpClient::Init( "
				"this : %p, "
				"[OK], "
				"mFd : %d, "
				"sendIp : %s, "
				"sendPort : %d, "
				"recvPort : %d "
				")",
				this,
				mFd,
				sendIp.c_str(),
				sendPort,
				recvPort
				);
	} else {
		LogAync(
				LOG_ERR_SYS,
				"UdpClient::Init( "
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
		Close();
	}

	return bFlag;
}

void UdpClient::Close() {
	if( mFd != -1 ) {
		close(mFd);
		mFd = -1;
	}
}

int UdpClient::SendData(const void *data, unsigned int len) {
	int sendSize = sendto(mFd, data, len, 0, (struct sockaddr *)&mSendSockAddr, sizeof(struct sockaddr_in));
//	LogAync(
//			LOG_STAT,
//			"UdpClient::SendData( "
//			"this : %p, "
//			"len : %d, "
//			"sendSize : %d "
//			")",
//			this,
//			len,
//			sendSize
//			);
	return sendSize;
}

} /* namespace mediaserver */
