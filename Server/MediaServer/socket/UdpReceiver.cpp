/*
 * UdpReceiver.cpp
 *
 *  Created on: 2019/08/26
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "UdpReceiver.h"

#include <common/LogManager.h>
#include <common/Math.h>
#include <common/CommonFunc.h>

namespace qpidnetwork {

UdpReceiver::UdpReceiver() {
	// TODO Auto-generated constructor stub
	mRecvIp = "";
	mRecvPort = -1;
	mFd = -1;
}

UdpReceiver::~UdpReceiver() {
	// TODO Auto-generated destructor stub
}

bool UdpReceiver::Init(const string& recvIp, int recvPort) {
	bool bFlag = true;

	mRecvIp = recvIp;
	mRecvPort = recvPort;

	// Check network address
	in_addr_t inAddrT = inet_addr(recvIp.c_str());
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
		mRecvSockAddr.sin_addr = inAddr;
		mRecvSockAddr.sin_family = PF_INET;
		mRecvSockAddr.sin_port = htons(mRecvPort);

        if ( bind(mFd, (struct sockaddr *)&mRecvSockAddr, sizeof(mRecvSockAddr)) < 0 ) {
        	bFlag = false;
        }
    }

	if( bFlag ) {
		LogAync(
				LOG_INFO,
				"UdpReceiver::Init, "
				"this:%p, "
				"[OK], "
				"fd:%d, "
				"recvIp:%s, "
				"recvPort:%d ",
				this,
				mFd,
				recvIp.c_str(),
				recvPort
				);
	} else {
		LogAync(
				LOG_ALERT,
				"UdpReceiver::Init, "
				"this:%p, "
				"[Fail], "
				"recvIp:%s, "
				"recvPort:%d ",
				this,
				recvIp.c_str(),
				recvPort
				);
		Close();
	}

	return bFlag;
}

void UdpReceiver::Shutdown() {
	if( mFd != -1 ) {
		LogAync(
				LOG_INFO,
				"UdpReceiver::Shutdown, "
				"this:%p, "
				"fd:%d ",
				this,
				mFd
				);
		shutdown(mFd, SHUT_RD);
	}
}

void UdpReceiver::Close() {
	if( mFd != -1 ) {
		LogAync(
				LOG_INFO,
				"UdpReceiver::Close, "
				"this:%p, "
				"fd:%d ",
				this,
				mFd
				);
		close(mFd);
		mFd = -1;
	}
}

int UdpReceiver::RecvData(void *buffer, unsigned int size) {
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	int recvSize = recvfrom(mFd, buffer, size, 0, (struct sockaddr *)&addr, &addr_len);

//	LogAync(
//			LOG_DEBUG,
//			"UdpReceiver::RecvData( "
//			"this:%p, "
//			"fd:%d, "
//			"size:%d, "
//			"recvSize:%d "
//			")",
//			this,
//			mFd,
//			size,
//			recvSize
//			);

	return recvSize;
}

} /* namespace qpidnetwork */
