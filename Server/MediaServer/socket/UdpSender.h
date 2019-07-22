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

namespace mediaserver {

class UdpSender: public SocketSender {
public:
	UdpSender();
	virtual ~UdpSender();

public:
	bool Init(const string& sendIp, int sendPort, int recvPort);
	void Close();
	int SendData(const void *data, unsigned int len);

private:
	// Socket
	string mSendIp;
	struct sockaddr_in mSendSockAddr;
	int mSendPort;
	int mRecvPort;
	int mFd;
};

} /* namespace mediaserver */

#endif /* SOCKET_UDPSENDER_H_ */
