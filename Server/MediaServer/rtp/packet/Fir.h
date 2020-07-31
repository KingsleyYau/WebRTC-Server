/*
 * Fir.h
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_PACKET_FIR_H_
#define RTP_PACKET_FIR_H_

#include "Psfb.h"

#include <vector>
using namespace std;

namespace mediaserver {
namespace rtcp {
class Fir: public Psfb {
public:
	static constexpr uint8_t kFeedbackMessageType = 4;
	struct Request {
		Request() :
				ssrc(0), seq_nr(0) {
		}
		Request(uint32_t ssrc, uint8_t seq_nr) :
				ssrc(ssrc), seq_nr(seq_nr) {
		}
		uint32_t ssrc;
		uint8_t seq_nr;
	};

	Fir();
	Fir(const Fir& fir);
	~Fir() override;

	// Parse assumes header is already parsed and validated.
	bool Parse(const CommonHeader& packet);

	void AddRequestTo(uint32_t ssrc, uint8_t seq_num) {
		items_.emplace_back(ssrc, seq_num);
	}
	const std::vector<Request>& requests() const {
		return items_;
	}

	size_t BlockLength() const override;

	bool Create(uint8_t* packet, size_t* index, size_t max_length) const
			override;

private:
	static constexpr size_t kFciLength = 8;

	// SSRC of media source is not used in FIR packet. Shadow base functions.
	uint32_t media_ssrc_ = 0;

	std::vector<Request> items_;
};
}
} /* namespace mediaserver */

#endif /* RTP_PACKET_FIR_H_ */
