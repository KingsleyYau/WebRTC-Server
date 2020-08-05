/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_API_UNITS_DATA_SIZE_H_
#define RTP_API_UNITS_DATA_SIZE_H_

#ifdef UNIT_TEST
#include <ostream>  // no-presubmit-check TODO(webrtc:8982)
#endif              // UNIT_TEST

#include <string>
#include <type_traits>

#include <rtp/base/units/unit_base.h>

namespace mediaserver {
// DataSize is a class represeting a count of bytes.
class DataSize final : public rtc_units_impl::RelativeUnit<DataSize> {
public:
	DataSize() = delete;
	static constexpr DataSize Infinity() {
		return PlusInfinity();
	}
	template<int64_t bytes>
	static constexpr DataSize Bytes() {
		return FromStaticValue<bytes>();
	}

	template<typename T>
	static DataSize bytes(T bytes) {
		static_assert(std::is_arithmetic<T>::value, "");
		return FromValue(bytes);
	}
	template<typename T = int64_t>
	T bytes() const {
		return ToValue<T>();
	}

	constexpr int64_t bytes_or(int64_t fallback_value) const {
		return ToValueOr(fallback_value);
	}

private:
	friend class rtc_units_impl::UnitBase<DataSize>;
	using RelativeUnit::RelativeUnit;
	static constexpr bool one_sided = true;
};

std::string ToString(DataSize value);
inline std::string ToLogString(DataSize value) {
	return ToString(value);
}

#ifdef UNIT_TEST
inline std::ostream& operator<<(  // no-presubmit-check TODO(webrtc:8982)
		std::ostream& stream,// no-presubmit-check TODO(webrtc:8982)
		DataSize value) {
	return stream << ToString(value);
}
#endif  // UNIT_TEST

}
  // namespace mediaserver

#endif  // RTP_API_UNITS_DATA_SIZE_H_
