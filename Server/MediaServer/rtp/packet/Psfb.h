/*
 * Psfb.h
 *
 *  Created on: 2020/07/15
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_PACKET_PSFB_H_
#define RTP_PACKET_PSFB_H_

#include "RtcpPacket.h"

namespace mediaserver {
namespace rtcp {
class Psfb : public RtcpPacket {
public:
	static constexpr uint8_t kPacketType = 206;
	static constexpr uint8_t kAfbMessageType = 15;

	Psfb() = default;
	virtual ~Psfb() override = default;

	uint32_t media_ssrc_ = 0;

protected:
	static constexpr size_t kCommonFeedbackLength = 8;
	void ParseCommonFeedback(const uint8_t* payload);
	void CreateCommonFeedback(uint8_t* payload) const;
};
}
} /* namespace mediaserver */

#endif /* RTP_PACKET_PSFB_H_ */
