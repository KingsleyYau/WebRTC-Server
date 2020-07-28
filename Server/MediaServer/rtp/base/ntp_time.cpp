/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include "ntp_time.h"

/**
 * https://tools.ietf.org/html/rfc3550#section-4
 * 一个UINT64表示一个NTP时间戳, 前32位表示整数部分, 后32位表示小数部分, 单位为秒
 */
namespace mediaserver {
static uint64_t kFractionsPerSecond = 0x100000000;
NtpTime::NtpTime() : value_(0) {
	// TODO Auto-generated constructor stub
}

NtpTime::NtpTime(uint32_t seconds, uint32_t fractions)
	: value_(seconds * kFractionsPerSecond + fractions) {
}

NtpTime::~NtpTime() {
	// TODO Auto-generated destructor stub
}

NtpTime::operator uint64_t() const {
	return value_;
}

void NtpTime::Set(uint32_t seconds, uint32_t fractions) {
	value_ = seconds * kFractionsPerSecond + fractions;
}

void NtpTime::Reset() {
	value_ = 0;
}

int64_t NtpTime::ToMs() const {
	static double kNtpFracPerMs = 4.294967296E6;  // 2^32 / 1000.
	const double frac_ms = static_cast<double>(fractions()) / kNtpFracPerMs;
	return 1000 * static_cast<int64_t>(seconds()) +
           static_cast<int64_t>(frac_ms + 0.5);
}

bool NtpTime::Valid() const {
	return value_ != 0;
}

uint32_t NtpTime::seconds() const {
    return (uint32_t)(value_ / kFractionsPerSecond);
}

uint32_t NtpTime::fractions() const {
    return (uint32_t)(value_ % kFractionsPerSecond);
}

} /* namespace mediaserver */
