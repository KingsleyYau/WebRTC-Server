/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_BASE_NTPTIME_H_
#define RTP_BASE_NTPTIME_H_

#include <unistd.h>
#include <stdint.h>

namespace qpidnetwork {

class NtpTime {
public:
	NtpTime();
	virtual ~NtpTime();
	NtpTime(uint32_t seconds, uint32_t fractions);
	explicit NtpTime(uint64_t value);
	NtpTime(const NtpTime&) = default;

	NtpTime& operator=(const NtpTime&) = default;
	explicit operator uint64_t() const;

	void Set(uint32_t seconds, uint32_t fractions);
	void Reset();

	int64_t ToMs() const;

	// NTP standard (RFC1305, section 3.1) explicitly state value 0 is invalid.
	bool Valid() const;

	uint32_t seconds() const;
	uint32_t fractions() const;
private:
	uint64_t value_;
};

} /* namespace qpidnetwork */

#endif /* RTP_BASE_NTPTIME_H_ */
