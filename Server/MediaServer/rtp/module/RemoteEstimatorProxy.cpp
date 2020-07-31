/*
 * RemoteEstimatorProxy.cpp
 *
 *  Created on: 2020/07/17
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "RemoteEstimatorProxy.h"

#include <common/CommonFunc.h>

namespace mediaserver {

// Impossible to request feedback older than what can be represented by 15 bits.
const int RemoteEstimatorProxy::kMaxNumberOfPackets = (1 << 15);

// The maximum allowed value for a timestamp in milliseconds. This is lower
// than the numerical limit since we often convert to microseconds.
static constexpr int64_t kMaxTimeMs = std::numeric_limits<int64_t>::max()
		/ 1000;

RemoteEstimatorProxy::RemoteEstimatorProxy() {
	// TODO Auto-generated constructor stub
	Reset();
}

RemoteEstimatorProxy::~RemoteEstimatorProxy() {
	// TODO Auto-generated destructor stub
}

void RemoteEstimatorProxy::Reset() {
	media_ssrc_ = 0;
	feedback_packet_count_ = 0;
	packet_arrival_times_.clear();
	window_seq_ = 0;
	last_send_time = 0;
}

void RemoteEstimatorProxy::RecvRtpPacket(int64_t arrival_time_ms,
		const RtpPacket* pkt) {
	if (arrival_time_ms < 0 || arrival_time_ms > kMaxTimeMs) {
		LogAync(LOG_WARNING, "RemoteEstimatorProxy::RecvRtpPacket( "
				"this : %p, "
				"[Arrival time out of bounds: %lld] "
				")", this, arrival_time_ms);
		return;
	}
	media_ssrc_ = pkt->ssrc_;
	int64_t seq = 0;
//	if (pkt->hasTransportSequenceNumber_) {
	if (false) {
//		seq = pkt->transport_sequence_number_;

		LogAync(LOG_DEBUG, "RemoteEstimatorProxy::RecvRtpPacket( "
				"this : %p, "
				"seq : %u, "
				"arrival_time_ms : %llu, "
				"window_seq_ : %lld, "
				"size : %d "
				")", this, seq, arrival_time_ms, window_seq_,
				packet_arrival_times_.size());

		if (window_seq_
				&& packet_arrival_times_.lower_bound(window_seq_)
						== packet_arrival_times_.end()) {
			// Start new feedback packet, cull old packets.
			for (auto it = packet_arrival_times_.begin();
					it != packet_arrival_times_.end() && it->first < seq
							&& arrival_time_ms - it->second >= 300;) {
//										>= send_config_.back_window->ms();) {
//
//				LogAync(LOG_DEBUG, "RemoteEstimatorProxy::RecvRtpPacket( "
//						"this : %p, "
//						"[Remove old packet], "
//						"seq : %u, "
//						"arrival_time_ms : %llu, "
//						"old seq : %u, "
//						"old_arrival_time_ms : %llu, "
//						"window_seq_ : %u, "
//						"begin_seq : %u, "
//						"end_seq : %u, "
//						"size : %d "
//						")", this, seq, arrival_time_ms, it->first, it->second,
//						window_seq_, packet_arrival_times_.begin()->first,
//						packet_arrival_times_.rbegin()->first,
//						packet_arrival_times_.size());

				it = packet_arrival_times_.erase(it);
			}
		}
		if (!window_seq_ || seq < window_seq_) {
			window_seq_ = seq;
		}

		// We are only interested in the first time a packet is received.
		if (packet_arrival_times_.find(seq) != packet_arrival_times_.end()) {
			LogAync(LOG_DEBUG, "RemoteEstimatorProxy::RecvRtpPacket( "
					"this : %p, "
					"[Duplicate packet], "
					"seq : %u, "
					"arrival_time_ms : %llu, "
					"window_seq_ : %u, "
					"begin_seq : %u, "
					"end_seq : %u, "
					"size : %d "
					")", this, seq, arrival_time_ms, window_seq_,
					packet_arrival_times_.begin()->first,
					packet_arrival_times_.end()->first,
					packet_arrival_times_.size());
			return;
		}

		packet_arrival_times_[seq] = arrival_time_ms;

		// Limit the range of sequence numbers to send feedback for.
		auto first_arrival_time_to_keep = packet_arrival_times_.lower_bound(
				packet_arrival_times_.rbegin()->first - kMaxNumberOfPackets);
		if (first_arrival_time_to_keep != packet_arrival_times_.begin()) {
			packet_arrival_times_.erase(packet_arrival_times_.begin(),
					first_arrival_time_to_keep);
//			if (send_periodic_feedback_) {
			// |packet_arrival_times_| cannot be empty since we just added one
			// element and the last element is not deleted.
			RTC_CHECK(!packet_arrival_times_.empty());
			window_seq_ = packet_arrival_times_.begin()->first;
//			}
		}
	}
}

bool RemoteEstimatorProxy::SendPeriodicFeedbacks(uint8_t* packet, size_t* index,
		size_t max_length) {
	// |window_seq_| is the first sequence number to include in the
	// current feedback packet. Some older may still be in the map, in case a
	// reordering happens and we need to retransmit them.
	bool bFlag = false;
	if (!window_seq_) {
		return bFlag;
	}

	int64_t now = getCurrentTime();
	if (!last_send_time) {
		last_send_time = now;
	}
	int64_t delta = now - last_send_time;
	if (delta < 50) {
		return bFlag;
	}

	for (auto begin_iterator = packet_arrival_times_.lower_bound(window_seq_);
			begin_iterator != packet_arrival_times_.cend(); begin_iterator =
					packet_arrival_times_.lower_bound(window_seq_)) {

		LogAync(LOG_DEBUG, "RemoteEstimatorProxy::SendPeriodicFeedbacks( "
				"this : %p, "
				"whole_begin_seq : %u, "
				"window_seq_ : %u, "
				"begin_seq : %u, "
				"end_seq : %u, "
				"size : %d "
				")", this, packet_arrival_times_.begin()->first, window_seq_,
				begin_iterator->first, packet_arrival_times_.rbegin()->first,
				packet_arrival_times_.size());

		rtcp::TransportFeedback feedback_packet;
		window_seq_ = BuildFeedbackPacket(feedback_packet_count_++, media_ssrc_,
				window_seq_, begin_iterator, packet_arrival_times_.end(),
				&feedback_packet);

		feedback_packet.Create(packet, index, max_length);
		bFlag = true;
		last_send_time = now;
		break;
		// Note: Don't erase items from packet_arrival_times_ after sending, in case
		// they need to be re-sent after a reordering. Removal will be handled
		// by OnPacketArrival once packets are too old.
	}

	return bFlag;
}

int64_t RemoteEstimatorProxy::BuildFeedbackPacket(uint8_t feedback_packet_count,
		uint32_t media_ssrc, int64_t base_sequence_number,
		std::map<int64_t, int64_t>::const_iterator begin_iterator,
		std::map<int64_t, int64_t>::const_iterator end_iterator,
		rtcp::TransportFeedback* feedback_packet) {
	RTC_CHECK(begin_iterator != end_iterator);

	// TODO(sprang): Measure receive times in microseconds and remove the
	// conversions below.
	feedback_packet->media_ssrc_ = media_ssrc;
	// Base sequence number is the expected first sequence number. This is known,
	// but we might not have actually received it, so the base time shall be the
	// time of the first received packet in the feedback.
	feedback_packet->SetBase(
			static_cast<uint16_t>(base_sequence_number & 0xFFFF),
			begin_iterator->second * 1000);
	feedback_packet->SetFeedbackSequenceNumber(feedback_packet_count);
	int64_t next_sequence_number = base_sequence_number;
	for (auto it = begin_iterator; it != end_iterator; ++it) {
		if (!feedback_packet->AddReceivedPacket(
				static_cast<uint16_t>(it->first & 0xFFFF), it->second * 1000)) {
			// If we can't even add the first seq to the feedback packet, we won't be
			// able to build it at all.
			RTC_CHECK(begin_iterator != it);

			// Could not add timestamp, feedback packet might be full. Return and
			// try again with a fresh packet.
			break;
		}
		next_sequence_number = it->first + 1;
	}
	return next_sequence_number;
}

} /* namespace mediaserver */
