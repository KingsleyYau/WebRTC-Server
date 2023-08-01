/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_INCLUDE_RTP_HEADER_EXTENSIONS_H_
#define RTP_INCLUDE_RTP_HEADER_EXTENSIONS_H_

#include <stddef.h>
#include <stdint.h>

#include <string>

#include <rtp/api/array_view.h>
#include <rtp/api/rtp_headers.h>
//#include <rtp/api/video/color_space.h>
//#include <rtp/api/video/video_content_type.h>
//#include <rtp/api/video/video_frame_marking.h>
//#include <rtp/api/video/video_rotation.h>
#include <rtp/api/video/video_timing.h>
#include <rtp/include/rtp_rtcp_defines.h>

namespace qpidnetwork {

class AbsoluteSendTime {
public:
	using value_type = uint32_t;
	static constexpr RTPExtensionType kId = kRtpExtensionAbsoluteSendTime;
	static constexpr uint8_t kValueSizeBytes = 3;
	static constexpr const char kUri[] =
			"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time";

	static bool Parse(qpidnetwork::ArrayView<const uint8_t> data,
			uint32_t* time_24bits);
	static size_t ValueSize(uint32_t time_24bits) {
		return kValueSizeBytes;
	}
	static bool Write(qpidnetwork::ArrayView<uint8_t> data,
			uint32_t time_24bits);

	static constexpr uint32_t MsTo24Bits(int64_t time_ms) {
		return static_cast<uint32_t>(((time_ms << 18) + 500) / 1000)
				& 0x00FFFFFF;
	}
};

class TransportSequenceNumber {
public:
	using value_type = uint16_t;
	static constexpr RTPExtensionType kId = kRtpExtensionTransportSequenceNumber;
	static constexpr uint8_t kValueSizeBytes = 2;
	static constexpr const char kUri[] = "http://www.ietf.org/id/"
			"draft-holmer-rmcat-transport-wide-cc-extensions-01";
	static bool Parse(qpidnetwork::ArrayView<const uint8_t> data,
			uint16_t* transport_sequence_number);
	static size_t ValueSize(uint16_t /*transport_sequence_number*/) {
		return kValueSizeBytes;
	}
	static bool Write(qpidnetwork::ArrayView<uint8_t> data,
			uint16_t transport_sequence_number);
};

class TransportSequenceNumberV2 {
public:
	static constexpr RTPExtensionType kId =
			kRtpExtensionTransportSequenceNumber02;
	static constexpr uint8_t kValueSizeBytes = 4;
	static constexpr uint8_t kValueSizeBytesWithoutFeedbackRequest = 2;
	static constexpr const char kUri[] =
			"http://www.webrtc.org/experiments/rtp-hdrext/transport-wide-cc-02";
	static bool Parse(qpidnetwork::ArrayView<const uint8_t> data,
			uint16_t* transport_sequence_number,
			absl::optional<FeedbackRequest>* feedback_request);
	static size_t ValueSize(uint16_t /*transport_sequence_number*/,
			const absl::optional<FeedbackRequest>& feedback_request) {
		return feedback_request ?
				kValueSizeBytes : kValueSizeBytesWithoutFeedbackRequest;
	}
	static bool Write(qpidnetwork::ArrayView<uint8_t> data,
			uint16_t transport_sequence_number,
			const absl::optional<FeedbackRequest>& feedback_request);

private:
	static constexpr uint16_t kIncludeTimestampsBit = 1 << 15;
};

class VideoTimingExtension {
public:
	using value_type = VideoSendTiming;
	static constexpr RTPExtensionType kId = kRtpExtensionVideoTiming;
	static constexpr uint8_t kValueSizeBytes = 13;
	static constexpr const char kUri[] =
			"http://www.webrtc.org/experiments/rtp-hdrext/video-timing";

	static bool Parse(qpidnetwork::ArrayView<const uint8_t> data,
			VideoSendTiming* timing);
	static size_t ValueSize(const VideoSendTiming&) {
		return kValueSizeBytes;
	}
	static bool Write(qpidnetwork::ArrayView<uint8_t> data,
			const VideoSendTiming& timing);

	static size_t ValueSize(uint16_t time_delta_ms, uint8_t idx) {
		return kValueSizeBytes;
	}
	// Writes only single time delta to position idx.
	static bool Write(qpidnetwork::ArrayView<uint8_t> data, uint16_t time_delta_ms,
			uint8_t idx);
};

}  // namespace qpidnetwork
#endif  // RTP_INCLUDE_RTP_HEADER_EXTENSIONS_H_
