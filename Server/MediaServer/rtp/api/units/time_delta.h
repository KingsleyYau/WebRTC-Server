/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_API_UNITS_TIMEDELTA_H_
#define RTP_API_UNITS_TIMEDELTA_H_

#include <cstdlib>
#include <string>
#include <type_traits>

#include <rtp/base/units/unit_base.h>

namespace mediaserver {

// TimeDelta represents the difference between two timestamps. Commonly this can
// be a duration. However since two Timestamps are not guaranteed to have the
// same epoch (they might come from different computers, making exact
// synchronisation infeasible), the duration covered by a TimeDelta can be
// undefined. To simplify usage, it can be constructed and converted to
// different units, specifically seconds (s), milliseconds (ms) and
// microseconds (us).
class TimeDelta final : public rtc_units_impl::RelativeUnit<TimeDelta> {
public:
	TimeDelta() = delete;
	template<int64_t seconds>
	static constexpr TimeDelta Seconds() {
		return FromStaticFraction<seconds, 1000000>();
	}
	template<int64_t ms>
	static constexpr TimeDelta Millis() {
		return FromStaticFraction<ms, 1000>();
	}
	template<int64_t us>
	static constexpr TimeDelta Micros() {
		return FromStaticValue<us>();
	}
	template<typename T>
	static TimeDelta seconds(T seconds) {
		static_assert(std::is_arithmetic<T>::value, "");
		return FromFraction<1000000>(seconds);
	}
	template<typename T>
	static TimeDelta ms(T milliseconds) {
		static_assert(std::is_arithmetic<T>::value, "");
		return FromFraction<1000>(milliseconds);
	}
	template<typename T>
	static TimeDelta us(T microseconds) {
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
	template<typename T = int64_t>
	T ns() const {
		return ToMultiple<1000, T>();
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

	TimeDelta Abs() const {
		return TimeDelta::us(std::abs(us()));
	}

private:
	friend class UnitBase<TimeDelta> ;
	using RelativeUnit::RelativeUnit;
	static constexpr bool one_sided = false;
};

}
// namespace mediaserver

#endif  // RTP_API_UNITS_TIMEDELTA_H_
