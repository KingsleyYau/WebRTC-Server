/*
 * Nack.h
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_PACKET_NACK_H_
#define RTP_PACKET_NACK_H_

#include "Rtpfb.h"

#include <vector>
using namespace std;

namespace qpidnetwork {
namespace rtcp {
class Nack: public Rtpfb {
public:
	static constexpr uint8_t kFeedbackMessageType = 1;
	Nack();
	Nack(const Nack&);
	~Nack() override;

	// Parse assumes header is already parsed and validated.
	bool Parse(const CommonHeader& packet);

	void SetPacketIdsWithStart(const uint16_t start_seq, size_t length);
	void SetPacketIds(const uint16_t* nack_list, size_t length);
	void SetPacketIds(std::vector<uint16_t> nack_list);
	const std::vector<uint16_t>& packet_ids() const {
		return packet_ids_;
	}

	size_t BlockLength() const override;

	bool Create(uint8_t* packet, size_t* index, size_t max_length) const
			override;

private:
	static constexpr size_t kNackItemLength = 4;
	struct PackedNack {
		uint16_t first_pid;
		uint16_t bitmask;
	};

	void Pack();    // Fills packed_ using packed_ids_. (used in SetPacketIds).
	void Unpack();  // Fills packet_ids_ using packed_. (used in Parse).

	std::vector<PackedNack> packed_;
	std::vector<uint16_t> packet_ids_;
};
}
} /* namespace qpidnetwork */

#endif /* RTP_PACKET_NACK_H_ */
