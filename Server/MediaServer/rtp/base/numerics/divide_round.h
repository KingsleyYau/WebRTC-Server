/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_BASE_NUMERICS_DIVIDE_ROUND_H_
#define RTP_BASE_NUMERICS_DIVIDE_ROUND_H_

#include <type_traits>

#include <rtp/base/checks.h>
#include <rtp/base/numerics/safe_compare.h>

namespace qpidnetwork {

template <typename Dividend, typename Divisor>
inline auto constexpr DivideRoundUp(Dividend dividend, Divisor divisor) {
  static_assert(std::is_integral<Dividend>(), "");
  static_assert(std::is_integral<Divisor>(), "");
//  RTC_DCHECK_GE(dividend, 0);
//  RTC_DCHECK_GT(divisor, 0);
//
//  auto quotient = dividend / divisor;
//  auto remainder = dividend % divisor;
//  return quotient + (remainder > 0 ? 1 : 0);
  return (dividend / divisor) + ((dividend % divisor) > 0 ? 1 : 0);
}

template <typename Dividend, typename Divisor>
inline auto constexpr DivideRoundToNearest(Dividend dividend, Divisor divisor) {
  static_assert(std::is_integral<Dividend>(), "");
  static_assert(std::is_integral<Divisor>(), "");
//  RTC_DCHECK_GE(dividend, 0);
//  RTC_DCHECK_GT(divisor, 0);
//
//  auto half_of_divisor = (divisor - 1) / 2;
//  auto quotient = dividend / divisor;
//  auto remainder = dividend % divisor;
  return (dividend / divisor) + (qpidnetwork::SafeGt((dividend % divisor), ((divisor - 1) / 2)) ? 1 : 0);
}

}  // namespace qpidnetwork

#endif  // RTP_BASE_NUMERICS_DIVIDE_ROUND_H_
