/*
 * RtpRawClient.h
 *
 *  Created on: 2019/07/03
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_RTPRAWCLIENT_H_
#define RTP_RTPRAWCLIENT_H_

#include "RtpClient.h"

#include <socket/UdpClient.h>

#include <string.h>
using namespace std;

namespace mediaserver {

class RtpRawClient: public RtpClient {
public:
	RtpRawClient();
	virtual ~RtpRawClient();

public:
	bool Init(const string& sendIp, int sendPort, int recvPort);

private:
	void SetSocketSender(SocketSender *sender);

private:
	UdpClient mUdpClient;
};

} /* namespace mediaserver */

#endif /* RTP_RTPRAWCLIENT_H_ */
