/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include <rtp/api/video/video_timing.h>

#include <rtp/api/array_view.h>
#include <rtp/base/numerics/safe_conversions.h>
#include <rtp/base/strings/string_builder.h>

namespace qpidnetwork {

uint16_t VideoSendTiming::GetDeltaCappedMs(int64_t base_ms, int64_t time_ms) {
	if (time_ms < base_ms) {
//		RTC_DLOG(LS_ERROR) << "Delta " << (time_ms - base_ms)
//				<< "ms expected to be positive";
	}
	return qpidnetwork::saturated_cast < uint16_t > (time_ms - base_ms);
}

TimingFrameInfo::TimingFrameInfo() :
		rtp_timestamp(0), capture_time_ms(-1), encode_start_ms(-1), encode_finish_ms(
				-1), packetization_finish_ms(-1), pacer_exit_ms(-1), network_timestamp_ms(
				-1), network2_timestamp_ms(-1), receive_start_ms(-1), receive_finish_ms(
				-1), decode_start_ms(-1), decode_finish_ms(-1), render_time_ms(
				-1), flags(VideoSendTiming::kNotTriggered) {
}

int64_t TimingFrameInfo::EndToEndDelay() const {
	return capture_time_ms >= 0 ? decode_finish_ms - capture_time_ms : -1;
}

bool TimingFrameInfo::IsLongerThan(const TimingFrameInfo& other) const {
	int64_t other_delay = other.EndToEndDelay();
	return other_delay == -1 || EndToEndDelay() > other_delay;
}

bool TimingFrameInfo::operator<(const TimingFrameInfo& other) const {
	return other.IsLongerThan(*this);
}

bool TimingFrameInfo::operator<=(const TimingFrameInfo& other) const {
	return !IsLongerThan(other);
}

bool TimingFrameInfo::IsOutlier() const {
	return !IsInvalid() && (flags & VideoSendTiming::kTriggeredBySize);
}

bool TimingFrameInfo::IsTimerTriggered() const {
	return !IsInvalid() && (flags & VideoSendTiming::kTriggeredByTimer);
}

bool TimingFrameInfo::IsInvalid() const {
	return flags == VideoSendTiming::kInvalid;
}

std::string TimingFrameInfo::ToString() const {
	if (IsInvalid()) {
		return "";
	}

	char buf[1024];
	qpidnetwork::SimpleStringBuilder sb(buf);

	sb << rtp_timestamp << ',' << capture_time_ms << ',' << encode_start_ms
			<< ',' << encode_finish_ms << ',' << packetization_finish_ms << ','
			<< pacer_exit_ms << ',' << network_timestamp_ms << ','
			<< network2_timestamp_ms << ',' << receive_start_ms << ','
			<< receive_finish_ms << ',' << decode_start_ms << ','
			<< decode_finish_ms << ',' << render_time_ms << ',' << IsOutlier()
			<< ',' << IsTimerTriggered();

	return sb.str();
}

}  // namespace qpidnetwork
