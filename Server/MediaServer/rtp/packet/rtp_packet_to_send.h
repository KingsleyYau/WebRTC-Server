/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_PACKET_TO_SEND_H_
#define RTP_PACKET_TO_SEND_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include <absl/types/optional.h>
#include <rtp/api/array_view.h>
#include <rtp/api/video/video_timing.h>
#include <rtp/include/rtp_header_extensions.h>
#include <rtp/packet/rtp_packet.h>

namespace mediaserver {
// Class to hold rtp packet with metadata for sender side.
class RtpPacketToSend: public RtpPacket {
public:
	enum class Type {
		kAudio,                   // Audio media packets.
		kVideo,                   // Video media packets.
		kRetransmission,      // RTX (usually) packets send as response to NACK.
		kForwardErrorCorrection,  // FEC packets.
		kPadding                  // RTX or plain padding sent to maintain BWE.
	};

	explicit RtpPacketToSend(const ExtensionManager* extensions);
	RtpPacketToSend(const ExtensionManager* extensions, size_t capacity);
	RtpPacketToSend(const RtpPacketToSend& packet);
	RtpPacketToSend(RtpPacketToSend&& packet);

	RtpPacketToSend& operator=(const RtpPacketToSend& packet);
	RtpPacketToSend& operator=(RtpPacketToSend&& packet);

	~RtpPacketToSend();

	// Time in local time base as close as it can to frame capture time.
	int64_t capture_time_ms() const {
		return capture_time_ms_;
	}

	void set_capture_time_ms(int64_t time) {
		capture_time_ms_ = time;
	}

	void set_packet_type(Type type) {
		packet_type_ = type;
	}
	absl::optional<Type> packet_type() const {
		return packet_type_;
	}

	// If this is a retransmission, indicates the sequence number of the original
	// media packet that this packet represents. If RTX is used this will likely
	// be different from SequenceNumber().
	void set_retransmitted_sequence_number(uint16_t sequence_number) {
		retransmitted_sequence_number_ = sequence_number;
	}
	absl::optional<uint16_t> retransmitted_sequence_number() {
		return retransmitted_sequence_number_;
	}

	void set_allow_retransmission(bool allow_retransmission) {
		allow_retransmission_ = allow_retransmission;
	}
	bool allow_retransmission() {
		return allow_retransmission_;
	}

	// Additional data bound to the RTP packet for use in application code,
	// outside of WebRTC.
	mediaserver::ArrayView<const uint8_t> application_data() const {
		return application_data_;
	}

	void set_application_data(mediaserver::ArrayView<const uint8_t> data) {
		application_data_.assign(data.begin(), data.end());
	}

	void set_packetization_finish_time_ms(int64_t time) {
		SetExtension<VideoTimingExtension>(
				VideoSendTiming::GetDeltaCappedMs(capture_time_ms_, time),
				VideoSendTiming::kPacketizationFinishDeltaOffset);
	}

	void set_pacer_exit_time_ms(int64_t time) {
		SetExtension<VideoTimingExtension>(
				VideoSendTiming::GetDeltaCappedMs(capture_time_ms_, time),
				VideoSendTiming::kPacerExitDeltaOffset);
	}

	void set_network_time_ms(int64_t time) {
		SetExtension<VideoTimingExtension>(
				VideoSendTiming::GetDeltaCappedMs(capture_time_ms_, time),
				VideoSendTiming::kNetworkTimestampDeltaOffset);
	}

	void set_network2_time_ms(int64_t time) {
		SetExtension<VideoTimingExtension>(
				VideoSendTiming::GetDeltaCappedMs(capture_time_ms_, time),
				VideoSendTiming::kNetwork2TimestampDeltaOffset);
	}

private:
	int64_t capture_time_ms_ = 0;
	absl::optional<Type> packet_type_;
	bool allow_retransmission_ = false;
	absl::optional<uint16_t> retransmitted_sequence_number_;
	std::vector<uint8_t> application_data_;
};

}  // namespace mediaserver
#endif  // RTP_PACKET_TO_SEND_H_
