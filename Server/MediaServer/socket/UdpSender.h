/*
 * UdpClient.h
 *
 *  Created on: 2019/07/02
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef SOCKET_UDPSENDER_H_
#define SOCKET_UDPSENDER_H_

#include "ISocketSender.h"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>
using namespace std;

namespace qpidnetwork {

class UdpSender:public SocketSender {
public:
	UdpSender();
	virtual ~UdpSender();

public:
	bool Init(const string& sendIp, int sendPort);
	void Close();
	int SendData(const void *buffer, unsigned int size);

private:
	// Socket
	string mSendIp;
	struct sockaddr_in mSendSockAddr;
	int mSendPort;
	int mFd;
};

} /* namespace qpidnetwork */

#endif /* SOCKET_UDPSENDER_H_ */
