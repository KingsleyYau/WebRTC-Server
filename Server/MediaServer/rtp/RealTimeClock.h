/*
 * RealTimeClock.h
 *
 *  Created on: 2019/12/25
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_REALTIMECLOCK_H_
#define RTP_REALTIMECLOCK_H_

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include <rtp/NtpTime.h>

// January 1970, in NTP seconds.
const uint32_t kNtpJan1970 = 2208988800UL;
// Magic NTP fractional unit.
const double kMagicNtpFractionalUnit = 4.294967296E+9;

namespace mediaserver {

class RealTimeClock {
public:
	static timeval CurrentTimeVal() {
		struct timeval tv;
		struct timezone tz;
		tz.tz_minuteswest = 0;
		tz.tz_dsttime = 0;
		gettimeofday(&tv, &tz);
		return tv;
	}

	// Retrieve an NTP absolute timestamp.
	static NtpTime CurrentNtpTime() {
		timeval tv = CurrentTimeVal();
		double microseconds_in_seconds;
		uint32_t seconds;
		Adjust(tv, &seconds, &microseconds_in_seconds);
		uint32_t fractions = static_cast<uint32_t>(
				microseconds_in_seconds * kMagicNtpFractionalUnit + 0.5);
		return NtpTime(seconds, fractions);
	}

	// Retrieve an NTP absolute timestamp in milliseconds.
	static int64_t CurrentNtpInMilliseconds() {
		timeval tv = CurrentTimeVal();
		uint32_t seconds;
		double microseconds_in_seconds;
		Adjust(tv, &seconds, &microseconds_in_seconds);
		return 1000 * static_cast<int64_t>(seconds) +
				static_cast<int64_t>(1000.0 * microseconds_in_seconds + 0.5);
	}

	static void Adjust(const timeval& tv,
                     uint32_t* adjusted_s,
                     double* adjusted_us_in_s) {
		*adjusted_s = tv.tv_sec + kNtpJan1970;
		*adjusted_us_in_s = tv.tv_usec / 1e6;

		if (*adjusted_us_in_s >= 1) {
			*adjusted_us_in_s -= 1;
			++*adjusted_s;
		} else if (*adjusted_us_in_s < -1) {
			*adjusted_us_in_s += 1;
			--*adjusted_s;
		}
	}
};

} /* namespace mediaserver */

#endif /* RTP_REALTIMECLOCK_H_ */
