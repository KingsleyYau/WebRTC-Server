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
	mFd = -1;
}

UdpSender::~UdpSender() {
	// TODO Auto-generated destructor stub
	Close();
}

bool UdpSender::Init(const string& sendIp, int sendPort) {
	bool bFlag = true;

	mSendIp = sendIp;
	mSendPort = sendPort;

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

    	int bOptval = 1;
    	if ( setsockopt(mFd, SOL_SOCKET, SO_REUSEADDR, &bOptval, sizeof(int)) ) {
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
				LOG_MSG,
				"UdpSender::Init( "
				"this : %p, "
				"[OK], "
				"fd : %d, "
				"sendIp : %s, "
				"sendPort : %d "
				")",
				this,
				mFd,
				sendIp.c_str(),
				sendPort
				);
	} else {
		LogAync(
				LOG_ERR_SYS,
				"UdpSender::Init( "
				"this : %p, "
				"[Fail], "
				"sendIp : %s, "
				"sendPort : %d "
				")",
				this,
				sendIp.c_str(),
				sendPort
				);
		Close();
	}

	return bFlag;
}

void UdpSender::Close() {
	LogAync(
			LOG_ERR_SYS,
			"UdpSender::Close( "
			"this : %p, "
			"fd : %d "
			")",
			this,
			mFd
			);

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
//			"fd : %d, "
//			"len : %d, "
//			"sendSize : %d "
//			")",
//			this,
//			mFd,
//			len,
//			sendSize
//			);
	return sendSize;
}

} /* namespace mediaserver */
