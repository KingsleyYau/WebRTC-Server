/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_BASE_NUMERICS_SEQUENCE_NUMBER_UTIL_H_
#define RTP_BASE_NUMERICS_SEQUENCE_NUMBER_UTIL_H_

#include <stdint.h>

#include <limits>
#include <type_traits>

#include <absl/types/optional.h>

#include <rtp/base/checks.h>
#include <rtp/base/numerics/mod_ops.h>

namespace mediaserver {

// Test if the sequence number |a| is ahead or at sequence number |b|.
//
// If |M| is an even number and the two sequence numbers are at max distance
// from each other, then the sequence number with the highest value is
// considered to be ahead.
template <typename T, T M>
inline typename std::enable_if<(M > 0), bool>::type AheadOrAt(T a, T b) {
  static_assert(std::is_unsigned<T>::value,
                "Type must be an unsigned integer.");
  const T maxDist = M / 2;
  if (!(M & 1) && MinDiff<T, M>(a, b) == maxDist)
    return b < a;
  return ForwardDiff<T, M>(b, a) <= maxDist;
}

template <typename T, T M>
inline typename std::enable_if<(M == 0), bool>::type AheadOrAt(T a, T b) {
  static_assert(std::is_unsigned<T>::value,
                "Type must be an unsigned integer.");
  const T maxDist = std::numeric_limits<T>::max() / 2 + T(1);
  if (a - b == maxDist)
    return b < a;
  return ForwardDiff(b, a) < maxDist;
}

template <typename T>
inline bool AheadOrAt(T a, T b) {
  return AheadOrAt<T, 0>(a, b);
}

// Test if the sequence number |a| is ahead of sequence number |b|.
//
// If |M| is an even number and the two sequence numbers are at max distance
// from each other, then the sequence number with the highest value is
// considered to be ahead.
template <typename T, T M = 0>
inline bool AheadOf(T a, T b) {
  static_assert(std::is_unsigned<T>::value,
                "Type must be an unsigned integer.");
  return a != b && AheadOrAt<T, M>(a, b);
}

// Comparator used to compare sequence numbers in a continuous fashion.
//
// WARNING! If used to sort sequence numbers of length M then the interval
//          covered by the sequence numbers may not be larger than floor(M/2).
template <typename T, T M = 0>
struct AscendingSeqNumComp {
  bool operator()(T a, T b) const { return AheadOf<T, M>(a, b); }
};

// Comparator used to compare sequence numbers in a continuous fashion.
//
// WARNING! If used to sort sequence numbers of length M then the interval
//          covered by the sequence numbers may not be larger than floor(M/2).
template <typename T, T M = 0>
struct DescendingSeqNumComp {
  bool operator()(T a, T b) const { return AheadOf<T, M>(b, a); }
};

// A sequence number unwrapper where the first unwrapped value equals the
// first value being unwrapped.
template <typename T, T M = 0>
class SeqNumUnwrapper {
  static_assert(
      std::is_unsigned<T>::value &&
          std::numeric_limits<T>::max() < std::numeric_limits<int64_t>::max(),
      "Type unwrapped must be an unsigned integer smaller than int64_t.");

 public:
  int64_t Unwrap(T value) {
    if (!last_value_) {
      last_unwrapped_ = {value};
    } else {
      last_unwrapped_ += ForwardDiff<T, M>(*last_value_, value);

      if (!AheadOrAt<T, M>(value, *last_value_)) {
        constexpr int64_t kBackwardAdjustment =
            M == 0 ? int64_t{std::numeric_limits<T>::max()} + 1 : M;
        last_unwrapped_ -= kBackwardAdjustment;
      }
    }

    last_value_ = value;
    return last_unwrapped_;
  }

 private:
  int64_t last_unwrapped_ = 0;
  absl::optional<T> last_value_;
};

}  // namespace mediaserver

#endif  // RTP_BASE_NUMERICS_SEQUENCE_NUMBER_UTIL_H_
