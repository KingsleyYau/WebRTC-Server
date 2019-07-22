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

#include <string.h>

#include <socket/UdpSender.h>

using namespace std;

namespace mediaserver {

class RtpRawClient: public RtpClient {
public:
	RtpRawClient();
	virtual ~RtpRawClient();

public:
	bool Init(const string& sendIp, int rtpSendPort, int rtpRecvPort);

private:
	void SetSocketSender(SocketSender *sender);

private:
	UdpSender mRtpSender;
	UdpSender mRtcpSender;
};

} /* namespace mediaserver */

#endif /* RTP_RTPRAWCLIENT_H_ */
