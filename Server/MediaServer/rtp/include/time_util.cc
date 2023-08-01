/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include <rtp/include/time_util.h>

#include <algorithm>

#include <rtp/base/checks.h>
#include <rtp/base/numerics/divide_round.h>
#include <rtp/base/time_utils.h>

namespace qpidnetwork {
namespace {

int64_t NtpOffsetMsCalledOnce() {
	constexpr int64_t kNtpJan1970Sec = 2208988800;
	int64_t clock_time = qpidnetwork::TimeMillis();
	int64_t utc_time = qpidnetwork::TimeUTCMillis();
	return utc_time - clock_time
			+ kNtpJan1970Sec * qpidnetwork::kNumMillisecsPerSec;
}

}  // namespace

int64_t NtpOffsetMs() {
	// Calculate the offset once.
	static int64_t ntp_offset_ms = NtpOffsetMsCalledOnce();
	return ntp_offset_ms;
}

NtpTime TimeMicrosToNtp(int64_t time_us) {
	// Since this doesn't return a wallclock time, but only NTP representation
	// of qpidnetwork::TimeMillis() clock, the exact offset doesn't matter.
	// To simplify conversions between NTP and RTP time, this offset is
	// limited to milliseconds in resolution.
	int64_t time_ntp_us = time_us + NtpOffsetMs() * 1000;
	RTC_DCHECK_GE(time_ntp_us, 0);  // Time before year 1900 is unsupported.

	// TODO(danilchap): Convert both seconds and fraction together using int128
	// when that type is easily available.
	// Currently conversion is done separetly for seconds and fraction of a second
	// to avoid overflow.

	// Convert seconds to uint32 through uint64 for well-defined cast.
	// Wrap around (will happen in 2036) is expected for ntp time.
	uint32_t ntp_seconds = static_cast<uint64_t>(time_ntp_us
			/ qpidnetwork::kNumMicrosecsPerSec);

	// Scale fractions of the second to ntp resolution.
	constexpr int64_t kNtpInSecond = 1LL << 32;
	int64_t us_fractions = time_ntp_us % qpidnetwork::kNumMicrosecsPerSec;
	uint32_t ntp_fractions = us_fractions * kNtpInSecond
			/ qpidnetwork::kNumMicrosecsPerSec;
	return NtpTime(ntp_seconds, ntp_fractions);
}

uint32_t SaturatedUsToCompactNtp(int64_t us) {
	constexpr uint32_t kMaxCompactNtp = 0xFFFFFFFF;
	constexpr int kCompactNtpInSecond = 0x10000;
	if (us <= 0)
		return 0;
	if (us
			>= kMaxCompactNtp * qpidnetwork::kNumMicrosecsPerSec
					/ kCompactNtpInSecond)
		return kMaxCompactNtp;
	// To convert to compact ntp need to divide by 1e6 to get seconds,
	// then multiply by 0x10000 to get the final result.
	// To avoid float operations, multiplication and division swapped.
	return DivideRoundToNearest(us * kCompactNtpInSecond,
			qpidnetwork::kNumMicrosecsPerSec);
}

int64_t CompactNtpRttToMs(uint32_t compact_ntp_interval) {
	// Interval to convert expected to be positive, e.g. rtt or delay.
	// Because interval can be derived from non-monotonic ntp clock,
	// it might become negative that is indistinguishable from very large values.
	// Since very large rtt/delay are less likely than non-monotonic ntp clock,
	// those values consider to be negative and convert to minimum value of 1ms.
	if (compact_ntp_interval > 0x80000000)
		return 1;
	// Convert to 64bit value to avoid multiplication overflow.
	int64_t value = static_cast<int64_t>(compact_ntp_interval);
	// To convert to milliseconds need to divide by 2^16 to get seconds,
	// then multiply by 1000 to get milliseconds. To avoid float operations,
	// multiplication and division swapped.
	int64_t ms = DivideRoundToNearest(value * 1000, 1 << 16);
	// Rtt value 0 considered too good to be true and increases to 1.
	return std::max<int64_t>(ms, 1);
}
}  // namespace qpidnetwork
