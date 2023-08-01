/*
 * Pli.h
 *
 *  Created on: 2020/07/15
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_PACKET_PLI_H_
#define RTP_PACKET_PLI_H_

#include "Psfb.h"

namespace qpidnetwork {
namespace rtcp {
// Picture loss indication (PLI) (RFC 4585).
class Pli : public Psfb {
public:
	static constexpr uint8_t kFeedbackMessageType = 1;

	Pli();
	~Pli() override;

	bool Parse(const CommonHeader& packet);

	size_t BlockLength() const override;
	bool Create(uint8_t* packet, size_t* index, size_t max_length) const override;
};
}
} /* namespace qpidnetwork */

#endif /* RTP_PACKET_PLI_H_ */
