/*
 * Remb.h
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_PACKET_REMB_H_
#define RTP_PACKET_REMB_H_

#include "Psfb.h"

#include <vector>
using namespace std;

namespace qpidnetwork {
namespace rtcp {
// Receiver Estimated Max Bitrate (REMB) (draft-alvestrand-rmcat-remb).
class Remb : public Psfb {
public:
	static constexpr size_t kMaxNumberOfSsrcs = 0xff;

	Remb();
	Remb(const Remb&);
	~Remb() override;

	// Parse assumes header is already parsed and validated.
	bool Parse(const CommonHeader& packet);

	bool SetSsrcs(std::vector<uint32_t> ssrcs);
	const std::vector<uint32_t>& ssrcs() const {
		return ssrcs_;
	}

	size_t BlockLength() const override;

	bool Create(uint8_t* packet, size_t* index, size_t max_length) const override;

	uint64_t bitrate_bps_;

private:
	static constexpr uint32_t kUniqueIdentifier = 0x52454D42; // 'R' 'E' 'M' 'B'.

	// SSRC of media source is not used in FIR packet. Shadow base functions.
	uint32_t media_ssrc_ = 0;

	std::vector<uint32_t> ssrcs_;
};
}
} /* namespace qpidnetwork */

#endif /* RTP_PACKET_REMB_H_ */
