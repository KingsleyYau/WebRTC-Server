/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_BASE_UNITS_UNIT_BASE_H_
#define RTP_BASE_UNITS_UNIT_BASE_H_

#include <stdint.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>

#include <rtp/base/checks.h>
#include <rtp/base/numerics/safe_conversions.h>

namespace qpidnetwork {
namespace rtc_units_impl {
// UnitBase is a base class for implementing custom value types with a specific
// unit. It provides type safety and commonly useful operations. The underlying
// storage is always an int64_t, it's up to the unit implementation to choose
// what scale it represents.
//
// It's used like:
// class MyUnit: public UnitBase<MyUnit> {...};
//
// Unit_T is the subclass representing the specific unit.
template<class Unit_T>
class UnitBase {
public:
	UnitBase() = delete;
	static constexpr Unit_T Zero() {
		return Unit_T(0);
	}
	static constexpr Unit_T PlusInfinity() {
		return Unit_T(PlusInfinityVal());
	}
	static constexpr Unit_T MinusInfinity() {
		return Unit_T(MinusInfinityVal());
	}

	constexpr bool IsZero() const {
		return value_ == 0;
	}
	constexpr bool IsFinite() const {
		return !IsInfinite();
	}
	constexpr bool IsInfinite() const {
		return value_ == PlusInfinityVal() || value_ == MinusInfinityVal();
	}
	constexpr bool IsPlusInfinity() const {
		return value_ == PlusInfinityVal();
	}
	constexpr bool IsMinusInfinity() const {
		return value_ == MinusInfinityVal();
	}

	constexpr bool operator==(const Unit_T& other) const {
		return value_ == other.value_;
	}
	constexpr bool operator!=(const Unit_T& other) const {
		return value_ != other.value_;
	}
	constexpr bool operator<=(const Unit_T& other) const {
		return value_ <= other.value_;
	}
	constexpr bool operator>=(const Unit_T& other) const {
		return value_ >= other.value_;
	}
	constexpr bool operator>(const Unit_T& other) const {
		return value_ > other.value_;
	}
	constexpr bool operator<(const Unit_T& other) const {
		return value_ < other.value_;
	}
	Unit_T RoundTo(const Unit_T& resolution) const {
		RTC_CHECK(IsFinite()); RTC_CHECK(resolution.IsFinite()); RTC_CHECK_GT(resolution.value_, 0);
		return Unit_T((value_ + resolution.value_ / 2) / resolution.value_)
				* resolution.value_;
	}
	Unit_T RoundUpTo(const Unit_T& resolution) const {
		RTC_CHECK(IsFinite()); RTC_CHECK(resolution.IsFinite()); RTC_CHECK_GT(resolution.value_, 0);
		return Unit_T((value_ + resolution.value_ - 1) / resolution.value_)
				* resolution.value_;
	}
	Unit_T RoundDownTo(const Unit_T& resolution) const {
		RTC_CHECK(IsFinite()); RTC_CHECK(resolution.IsFinite()); RTC_CHECK_GT(resolution.value_, 0);
		return Unit_T(value_ / resolution.value_) * resolution.value_;
	}

protected:
	template<int64_t value>
	static constexpr Unit_T FromStaticValue() {
		static_assert(value >= 0 || !Unit_T::one_sided, "");
		static_assert(value > MinusInfinityVal(), "");
		static_assert(value < PlusInfinityVal(), "");
		return Unit_T(value);
	}

	template<int64_t fraction_value, int64_t Denominator>
	static constexpr Unit_T FromStaticFraction() {
		static_assert(fraction_value >= 0 || !Unit_T::one_sided, "");
		static_assert(fraction_value > MinusInfinityVal() / Denominator, "");
		static_assert(fraction_value < PlusInfinityVal() / Denominator, "");
		return Unit_T(fraction_value * Denominator);
	}

	template<typename T,
			typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
	static Unit_T FromValue(T value) {
		if (Unit_T::one_sided)
			RTC_CHECK_GE(value, 0); RTC_CHECK_GT(value, MinusInfinityVal()); RTC_CHECK_LT(value, PlusInfinityVal());
		return Unit_T(dchecked_cast<int64_t>(value));
	}
	template<typename T, typename std::enable_if<
			std::is_floating_point<T>::value>::type* = nullptr>
	static Unit_T FromValue(T value) {
		if (value == std::numeric_limits<T>::infinity()) {
			return PlusInfinity();
		} else if (value == -std::numeric_limits<T>::infinity()) {
			return MinusInfinity();
		} else {
			RTC_CHECK(!std::isnan(value));
			return FromValue(dchecked_cast<int64_t>(value));
		}
	}

	template<int64_t Denominator, typename T, typename std::enable_if<
			std::is_integral<T>::value>::type* = nullptr>
	static Unit_T FromFraction(T value) {
		if (Unit_T::one_sided)
			RTC_CHECK_GE(value, 0); RTC_CHECK_GT(value, MinusInfinityVal() / Denominator); RTC_CHECK_LT(value, PlusInfinityVal() / Denominator);
		return Unit_T(dchecked_cast<int64_t>(value * Denominator));
	}
	template<int64_t Denominator, typename T, typename std::enable_if<
			std::is_floating_point<T>::value>::type* = nullptr>
	static Unit_T FromFraction(T value) {
		return FromValue(value * Denominator);
	}

	template<typename T = int64_t>
	typename std::enable_if<std::is_integral<T>::value, T>::type ToValue() const {
		RTC_CHECK(IsFinite());
		return dchecked_cast<T>(value_);
	}
	template<typename T>
constexpr	typename std::enable_if<std::is_floating_point<T>::value, T>::type
	ToValue() const {
		return IsPlusInfinity()
		? std::numeric_limits<T>::infinity()
		: IsMinusInfinity() ? -std::numeric_limits<T>::infinity()
		: value_;
	}
	template <typename T>
	constexpr T ToValueOr(T fallback_value) const {
		return IsFinite() ? value_ : fallback_value;
	}

	template <int64_t Denominator, typename T = int64_t>
	typename std::enable_if<std::is_integral<T>::value, T>::type ToFraction()
	const {
		RTC_CHECK(IsFinite());
		if (Unit_T::one_sided) {
			return dchecked_cast<T>(
					DivRoundPositiveToNearest(value_, Denominator));
		} else {
			return dchecked_cast<T>(DivRoundToNearest(value_, Denominator));
		}
	}
	template <int64_t Denominator, typename T>
	constexpr typename std::enable_if<std::is_floating_point<T>::value, T>::type
	ToFraction() const {
		return ToValue<T>() * (1 / static_cast<T>(Denominator));
	}

	template <int64_t Denominator>
	constexpr int64_t ToFractionOr(int64_t fallback_value) const {
		return IsFinite() ? Unit_T::one_sided
		? DivRoundPositiveToNearest(value_, Denominator)
		: DivRoundToNearest(value_, Denominator)
		: fallback_value;
	}

	template <int64_t Factor, typename T = int64_t>
	typename std::enable_if<std::is_integral<T>::value, T>::type ToMultiple()
	const {
		RTC_CHECK_GE(ToValue(), std::numeric_limits<T>::min() / Factor);
		RTC_CHECK_LE(ToValue(), std::numeric_limits<T>::max() / Factor);
		return dchecked_cast<T>(ToValue() * Factor);
	}
	template <int64_t Factor, typename T>
	constexpr typename std::enable_if<std::is_floating_point<T>::value, T>::type
	ToMultiple() const {
		return ToValue<T>() * Factor;
	}

	explicit constexpr UnitBase(int64_t value) : value_(value) {}

private:
	template <class RelativeUnit_T>
	friend class RelativeUnit;

	static inline constexpr int64_t PlusInfinityVal() {
		return std::numeric_limits<int64_t>::max();
	}
	static inline constexpr int64_t MinusInfinityVal() {
		return std::numeric_limits<int64_t>::min();
	}

	Unit_T& AsSubClassRef() {return reinterpret_cast<Unit_T&>(*this);}
	constexpr const Unit_T& AsSubClassRef() const {
		return reinterpret_cast<const Unit_T&>(*this);
	}
	// Assumes that n >= 0 and d > 0.
	static constexpr int64_t DivRoundPositiveToNearest(int64_t n, int64_t d) {
		return (n + d / 2) / d;
	}
	// Assumes that d > 0.
	static constexpr int64_t DivRoundToNearest(int64_t n, int64_t d) {
		return (n + (n >= 0 ? d / 2 : -d / 2)) / d;
	}

	int64_t value_;
};

// Extends UnitBase to provide operations for relative units, that is, units
// that have a meaningful relation between values such that a += b is a
// sensible thing to do. For a,b <- same unit.
template<class Unit_T>
class RelativeUnit: public UnitBase<Unit_T> {
public:
	Unit_T Clamped(Unit_T min_value, Unit_T max_value) const {
		return std::max(min_value,
				std::min(UnitBase<Unit_T>::AsSubClassRef(), max_value));
	}
	void Clamp(Unit_T min_value, Unit_T max_value) {
		*this = Clamped(min_value, max_value);
	}
	Unit_T operator+(const Unit_T other) const {
		if (this->IsPlusInfinity() || other.IsPlusInfinity()) {
			RTC_CHECK(!this->IsMinusInfinity()); RTC_CHECK(!other.IsMinusInfinity());
			return this->PlusInfinity();
		} else if (this->IsMinusInfinity() || other.IsMinusInfinity()) {
			RTC_CHECK(!this->IsPlusInfinity()); RTC_CHECK(!other.IsPlusInfinity());
			return this->MinusInfinity();
		}
		return UnitBase<Unit_T>::FromValue(this->ToValue() + other.ToValue());
	}
	Unit_T operator-(const Unit_T other) const {
		if (this->IsPlusInfinity() || other.IsMinusInfinity()) {
			RTC_CHECK(!this->IsMinusInfinity()); RTC_CHECK(!other.IsPlusInfinity());
			return this->PlusInfinity();
		} else if (this->IsMinusInfinity() || other.IsPlusInfinity()) {
			RTC_CHECK(!this->IsPlusInfinity()); RTC_CHECK(!other.IsMinusInfinity());
			return this->MinusInfinity();
		}
		return UnitBase<Unit_T>::FromValue(this->ToValue() - other.ToValue());
	}
	Unit_T& operator+=(const Unit_T other) {
		*this = *this + other;
		return this->AsSubClassRef();
	}
	Unit_T& operator-=(const Unit_T other) {
		*this = *this - other;
		return this->AsSubClassRef();
	}
	constexpr double operator/(const Unit_T other) const {
		return UnitBase<Unit_T>::template ToValue<double>()
				/ other.template ToValue<double>();
	}
	template<typename T>
	typename std::enable_if<std::is_arithmetic<T>::value, Unit_T>::type operator/(
			const T& scalar) const {
		return UnitBase<Unit_T>::FromValue(
				std::round(
						UnitBase<Unit_T>::template ToValue<int64_t>() / scalar));
	}
	Unit_T operator*(const double scalar) const {
		return UnitBase<Unit_T>::FromValue(std::round(this->ToValue() * scalar));
	}
	Unit_T operator*(const int64_t scalar) const {
		return UnitBase<Unit_T>::FromValue(this->ToValue() * scalar);
	}
	Unit_T operator*(const int32_t scalar) const {
		return UnitBase<Unit_T>::FromValue(this->ToValue() * scalar);
	}

protected:
	using UnitBase<Unit_T>::UnitBase;
};

template<class Unit_T>
inline Unit_T operator*(const double scalar, const RelativeUnit<Unit_T> other) {
	return other * scalar;
}
template<class Unit_T>
inline Unit_T operator*(const int64_t scalar,
		const RelativeUnit<Unit_T> other) {
	return other * scalar;
}
template<class Unit_T>
inline Unit_T operator*(const int32_t& scalar,
		const RelativeUnit<Unit_T> other) {
	return other * scalar;
}
}  // namespace rtc_units_impl
}  // namespace qpidnetwork

#endif  // RTP_BASE_UNITS_UNIT_BASE_H_
