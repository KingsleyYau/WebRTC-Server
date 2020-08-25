/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_BASE_NUMERICS_SAFECONVERSIONS_H_
#define RTP_BASE_NUMERICS_SAFECONVERSIONS_H_

#include <limits>

#include <rtp/base/checks.h>
#include <rtp/base/numerics/safe_conversions_impl.h>

namespace mediaserver {

// Convenience function that returns true if the supplied value is in range
// for the destination type.
template<typename Dst, typename Src>
inline constexpr bool IsValueInRangeForNumericType(Src value) {
	return RangeCheck<Dst>(value) == TYPE_VALID;
}

// checked_cast<> and dchecked_cast<> are analogous to static_cast<> for
// numeric types, except that they [D]CHECK that the specified numeric
// conversion will not overflow or underflow. NaN source will always trigger
// the [D]CHECK.
template<typename Dst, typename Src>
inline constexpr Dst checked_cast(Src value) {
	RTC_CHECK(IsValueInRangeForNumericType<Dst>(value));
	return static_cast<Dst>(value);
}
template<typename Dst, typename Src>
inline constexpr Dst dchecked_cast(Src value) {
	RTC_CHECK(IsValueInRangeForNumericType<Dst>(value));
	return static_cast<Dst>(value);
}

// saturated_cast<> is analogous to static_cast<> for numeric types, except
// that the specified numeric conversion will saturate rather than overflow or
// underflow. NaN assignment to an integral will trigger a RTC_CHECK condition.
template<typename Dst, typename Src>
inline Dst saturated_cast(Src value) {
	// Optimization for floating point values, which already saturate.
	if (std::numeric_limits<Dst>::is_iec559)
		return static_cast<Dst>(value);

	switch (RangeCheck<Dst>(value)) {
	case TYPE_VALID:
		return static_cast<Dst>(value);

	case TYPE_UNDERFLOW:
		return std::numeric_limits<Dst>::min();

	case TYPE_OVERFLOW:
		return std::numeric_limits<Dst>::max();

		// Should fail only on attempting to assign NaN to a saturated integer.
	case TYPE_INVALID:
//      FATAL();
		return std::numeric_limits<Dst>::max();
	}

//  FATAL();
	return static_cast<Dst>(value);
}

}  // namespace mediaserver

#endif  // #ifndef RTP_BASE_NUMERICS_SAFECONVERSIONS_H_

