/*
 * UdpSender.cpp
 *
 *  Created on: 2019/07/02
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include <common/LogManager.h>
#include <common/Math.h>
#include <common/CommonFunc.h>
#include "UdpSender.h"

namespace mediaserver {

UdpSender::UdpSender() {
	// TODO Auto-generated constructor stub
	mSendIp = "";
	mSendPort = -1;
	mRecvPort = -1;
	mFd = -1;
}

UdpSender::~UdpSender() {
	// TODO Auto-generated destructor stub
}

bool UdpSender::Init(const string& sendIp, int sendPort, int recvPort) {
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
				"UdpSender::Init( "
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
				"UdpSender::Init( "
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

void UdpSender::Close() {
	if( mFd != -1 ) {
		close(mFd);
		mFd = -1;
	}
}

int UdpSender::SendData(const void *data, unsigned int len) {
	int sendSize = sendto(mFd, data, len, 0, (struct sockaddr *)&mSendSockAddr, sizeof(struct sockaddr_in));
//	LogAync(
//			LOG_STAT,
//			"UdpSender::SendData( "
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
