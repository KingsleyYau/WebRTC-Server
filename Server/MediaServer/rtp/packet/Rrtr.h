/*
 * Rrtr.h
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_PACKET_RRTR_H_
#define RTP_PACKET_RRTR_H_

#include <stddef.h>
#include <stdint.h>

#include <rtp/base/ntp_time.h>

namespace qpidnetwork {
namespace rtcp {
class Rrtr {
public:
	static const uint8_t kBlockType = 4;
	static const uint16_t kBlockLength = 2;
	static const size_t kLength = 4 * (kBlockLength + 1);  // 12

	Rrtr() {
	}
	Rrtr(const Rrtr&) = default;
	~Rrtr() {
	}

	Rrtr& operator=(const Rrtr&) = default;

	void Parse(const uint8_t* buffer);

	// Fills buffer with the Rrtr.
	// Consumes Rrtr::kLength bytes.
	void Create(uint8_t* buffer) const;

	void SetNtp(NtpTime ntp) {
		ntp_ = ntp;
	}

	NtpTime ntp() const {
		return ntp_;
	}

private:
	NtpTime ntp_;
};
}
}  // namespace qpidnetwork
#endif  // RTP_PACKET_RRTR_H_
