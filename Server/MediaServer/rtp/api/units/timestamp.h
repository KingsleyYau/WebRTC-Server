/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_API_UNITS_TIMESTAMP_H_
#define RTP_API_UNITS_TIMESTAMP_H_

#ifdef UNIT_TEST
#include <ostream>  // no-presubmit-check TODO(webrtc:8982)
#endif              // UNIT_TEST

#include <string>
#include <type_traits>

#include <rtp/api/units/time_delta.h>
#include <rtp/base/checks.h>

namespace qpidnetwork {
// Timestamp represents the time that has passed since some unspecified epoch.
// The epoch is assumed to be before any represented timestamps, this means that
// negative values are not valid. The most notable feature is that the
// difference of two Timestamps results in a TimeDelta.
class Timestamp final : public rtc_units_impl::UnitBase<Timestamp> {
public:
	Timestamp() = delete;

	template<int64_t seconds>
	static constexpr Timestamp Seconds() {
		return FromStaticFraction<seconds, 1000000>();
	}
	template<int64_t ms>
	static constexpr Timestamp Millis() {
		return FromStaticFraction<ms, 1000>();
	}
	template<int64_t us>
	static constexpr Timestamp Micros() {
		return FromStaticValue<us>();
	}

	template<typename T>
	static Timestamp seconds(T seconds) {
		static_assert(std::is_arithmetic<T>::value, "");
		return FromFraction<1000000>(seconds);
	}
	template<typename T>
	static Timestamp ms(T milliseconds) {
		static_assert(std::is_arithmetic<T>::value, "");
		return FromFraction<1000>(milliseconds);
	}
	template<typename T>
	static Timestamp us(T microseconds) {
		static_assert(std::is_arithmetic<T>::value, "");
		return FromValue(microseconds);
	}
	template<typename T = int64_t>
	T seconds() const {
		return ToFraction<1000000, T>();
	}
	template<typename T = int64_t>
	T ms() const {
		return ToFraction<1000, T>();
	}
	template<typename T = int64_t>
	T us() const {
		return ToValue<T>();
	}

	constexpr int64_t seconds_or(int64_t fallback_value) const {
		return ToFractionOr<1000000>(fallback_value);
	}
	constexpr int64_t ms_or(int64_t fallback_value) const {
		return ToFractionOr<1000>(fallback_value);
	}
	constexpr int64_t us_or(int64_t fallback_value) const {
		return ToValueOr(fallback_value);
	}

	Timestamp operator+(const TimeDelta delta) const {
		if (IsPlusInfinity() || delta.IsPlusInfinity()) {
			RTC_DCHECK(!IsMinusInfinity());RTC_DCHECK(!delta.IsMinusInfinity());
			return PlusInfinity();
		} else if (IsMinusInfinity() || delta.IsMinusInfinity()) {
			RTC_DCHECK(!IsPlusInfinity());RTC_DCHECK(!delta.IsPlusInfinity());
			return MinusInfinity();
		}
		return Timestamp::us(us() + delta.us());
	}
	Timestamp operator-(const TimeDelta delta) const {
		if (IsPlusInfinity() || delta.IsMinusInfinity()) {
			RTC_DCHECK(!IsMinusInfinity());RTC_DCHECK(!delta.IsPlusInfinity());
			return PlusInfinity();
		} else if (IsMinusInfinity() || delta.IsPlusInfinity()) {
			RTC_DCHECK(!IsPlusInfinity());RTC_DCHECK(!delta.IsMinusInfinity());
			return MinusInfinity();
		}
		return Timestamp::us(us() - delta.us());
	}
	TimeDelta operator-(const Timestamp other) const {
		if (IsPlusInfinity() || other.IsMinusInfinity()) {
			RTC_DCHECK(!IsMinusInfinity());RTC_DCHECK(!other.IsPlusInfinity());
			return TimeDelta::PlusInfinity();
		} else if (IsMinusInfinity() || other.IsPlusInfinity()) {
			RTC_DCHECK(!IsPlusInfinity());RTC_DCHECK(!other.IsMinusInfinity());
			return TimeDelta::MinusInfinity();
		}
		return TimeDelta::us(us() - other.us());
	}
	Timestamp& operator-=(const TimeDelta delta) {
		*this = *this - delta;
		return *this;
	}
	Timestamp& operator+=(const TimeDelta delta) {
		*this = *this + delta;
		return *this;
	}

private:
	friend class UnitBase<Timestamp> ;
	using UnitBase::UnitBase;
	static constexpr bool one_sided = true;
};

//std::string ToString(Timestamp value);
//inline std::string ToLogString(Timestamp value) {
//  return ToString(value);
//}

#ifdef UNIT_TEST
inline std::ostream& operator<<(  // no-presubmit-check TODO(webrtc:8982)
		std::ostream& stream,// no-presubmit-check TODO(webrtc:8982)
		Timestamp value) {
	return stream << ToString(value);
}
#endif  // UNIT_TEST

}
  // namespace qpidnetwork

#endif  // RTP_API_UNITS_TIMESTAMP_H_
