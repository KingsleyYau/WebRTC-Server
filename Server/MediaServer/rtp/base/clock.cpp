/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include "clock.h"

#include <sys/time.h>
#include <time.h>

#include <rtp/base/rw_lock_wrapper.h>
#include <rtp/base/time_utils.h>

namespace qpidnetwork {

class RealTimeClock: public Clock {
	Timestamp CurrentTime() override {
		return Timestamp::us(TimeMicros());
	}
	// Return a timestamp in milliseconds relative to some arbitrary source; the
	// source is fixed for this clock.
	int64_t TimeInMilliseconds() override {
		return TimeMillis();
	}

	// Return a timestamp in microseconds relative to some arbitrary source; the
	// source is fixed for this clock.
	int64_t TimeInMicroseconds() override {
		return TimeMicros();
	}

	// Retrieve an NTP absolute timestamp.
	NtpTime CurrentNtpTime() override {
		timeval tv = CurrentTimeVal();
		double microseconds_in_seconds;
		uint32_t seconds;
		Adjust(tv, &seconds, &microseconds_in_seconds);
		uint32_t fractions = static_cast<uint32_t>(microseconds_in_seconds
				* kMagicNtpFractionalUnit + 0.5);
		return NtpTime(seconds, fractions);
	}

	// Retrieve an NTP absolute timestamp in milliseconds.
	int64_t CurrentNtpInMilliseconds() override {
		timeval tv = CurrentTimeVal();
		uint32_t seconds;
		double microseconds_in_seconds;
		Adjust(tv, &seconds, &microseconds_in_seconds);
		return 1000 * static_cast<int64_t>(seconds)
				+ static_cast<int64_t>(1000.0 * microseconds_in_seconds + 0.5);
	}

protected:
	virtual timeval CurrentTimeVal() = 0;

	static void Adjust(const timeval& tv, uint32_t* adjusted_s,
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

class UnixRealTimeClock: public RealTimeClock {
public:
	UnixRealTimeClock() {
	}

	~UnixRealTimeClock() override {
	}

protected:
	timeval CurrentTimeVal() override {
		struct timeval tv;
		struct timezone tz;
		tz.tz_minuteswest = 0;
		tz.tz_dsttime = 0;
		gettimeofday(&tv, &tz);
		return tv;
	}
};

Clock* Clock::GetRealTimeClock() {
	static Clock* const clock = new UnixRealTimeClock();
	return clock;
}

SimulatedClock::SimulatedClock(int64_t initial_time_us) :
		SimulatedClock(Timestamp::us(initial_time_us)) {
}

SimulatedClock::SimulatedClock(Timestamp initial_time) :
		time_(initial_time), lock_(RWLockWrapper::CreateRWLock()) {
}

SimulatedClock::~SimulatedClock() {
}

Timestamp SimulatedClock::CurrentTime() {
	ReadLockScoped synchronize(*lock_);
	return time_;
}

NtpTime SimulatedClock::CurrentNtpTime() {
	int64_t now_ms = TimeInMilliseconds();
	uint32_t seconds = (now_ms / 1000) + kNtpJan1970;
	uint32_t fractions = static_cast<uint32_t>((now_ms % 1000)
			* kMagicNtpFractionalUnit / 1000);
	return NtpTime(seconds, fractions);
}

int64_t SimulatedClock::CurrentNtpInMilliseconds() {
	return TimeInMilliseconds() + 1000 * static_cast<int64_t>(kNtpJan1970);
}

void SimulatedClock::AdvanceTimeMilliseconds(int64_t milliseconds) {
	AdvanceTime(TimeDelta::ms(milliseconds));
}

void SimulatedClock::AdvanceTimeMicroseconds(int64_t microseconds) {
	AdvanceTime(TimeDelta::us(microseconds));
}

void SimulatedClock::AdvanceTime(TimeDelta delta) {
	WriteLockScoped synchronize(*lock_);
	time_ += delta;
}

}  // namespace qpidnetwork
