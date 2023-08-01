/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_PACKET_RTPFB_H_
#define RTP_PACKET_RTPFB_H_

#include "RtcpPacket.h"

namespace qpidnetwork {
namespace rtcp {
class Rtpfb: public RtcpPacket {
public:
	static constexpr uint8_t kPacketType = 205;

	Rtpfb() = default;
	virtual ~Rtpfb() override = default;

	uint32_t media_ssrc_ = 0;

protected:
	static constexpr size_t kCommonFeedbackLength = 8;
	void ParseCommonFeedback(const uint8_t* payload);
	void CreateCommonFeedback(uint8_t* payload) const;
};
}
} /* namespace qpidnetwork */

#endif /* RTP_PACKET_RTPFB_H_ */
