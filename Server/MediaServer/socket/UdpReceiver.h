/*
 * UdpReceiver.h
 *
 *  Created on: 2019/08/26
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef SOCKET_UDPRECEIVER_H_
#define SOCKET_UDPRECEIVER_H_

#include "ISocketReceiver.h"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>
using namespace std;

namespace qpidnetwork {

class UdpReceiver:public SocketReceiver {
public:
	UdpReceiver();
	virtual ~UdpReceiver();

	bool Init(const string& recvIp, int recvPort);
	void Close();
	void Shutdown();
	int RecvData(void *data, unsigned int len);

private:
	// Socket
	string mRecvIp;
	struct sockaddr_in mRecvSockAddr;
	int mRecvPort;
	int mFd;
};

} /* namespace qpidnetwork */

#endif /* SOCKET_UDPRECEIVER_H_ */
