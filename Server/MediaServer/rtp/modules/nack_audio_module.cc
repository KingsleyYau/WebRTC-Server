/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <rtp/modules/nack_audio_module.h>

#include <assert.h>

#include <cstdint>
#include <utility>

#include <rtp/base/checks.h>

#include <include/CommonHeader.h>
#include <common/LogManager.h>

namespace qpidnetwork {
namespace {

const int kDefaultSampleRateKhz = 48;
const int kDefaultPacketSizeMs = 20;

}  // namespace

NackAudioModule::NackAudioModule(Clock* clock, int nack_threshold_packets) :
		clock_(clock), nack_threshold_packets_(nack_threshold_packets), sequence_num_last_received_rtp_(
				0), timestamp_last_received_rtp_(0), any_rtp_received_(false), sequence_num_first_received_rtp_(
				0), timestamp_first_received_rtp_(0), sample_rate_khz_(
				kDefaultSampleRateKhz), samples_per_packet_(
				sample_rate_khz_ * kDefaultPacketSizeMs), time_first_received_rtp_(
				Timestamp::ms(0)), time_last_check_rtp_(Timestamp::ms(0)), max_nack_list_size_(
				kNackListSizeLimit) {
}

NackAudioModule::~NackAudioModule() = default;

NackAudioModule* NackAudioModule::Create(Clock* clock,
		int nack_threshold_packets) {
	return new NackAudioModule(clock, nack_threshold_packets);
}

void NackAudioModule::UpdateSampleRate(int sample_rate_hz) {
	if (sample_rate_hz > 0) {
		sample_rate_khz_ = sample_rate_hz / 1000;
	}
}

void NackAudioModule::UpdateLastReceivedPacket(uint16_t sequence_number,
		uint32_t timestamp) {
	// Just record the value of sequence number and timestamp if this is the
	// first packet.
	if (!any_rtp_received_) {
		sequence_num_last_received_rtp_ = sequence_number;
		timestamp_last_received_rtp_ = timestamp;
		any_rtp_received_ = true;
		sequence_num_first_received_rtp_ = sequence_number;
		timestamp_first_received_rtp_ = timestamp;
		time_first_received_rtp_ = clock_->CurrentTime();
		return;
	}

	if (sequence_number == sequence_num_last_received_rtp_)
		return;

	// Received RTP should not be in the list.
	nack_list_.erase(sequence_number);

	UpdateEstimatedPlayoutTime();

	// If this is an old sequence number, no more action is required, return.
	if (IsNewerSequenceNumber(sequence_num_last_received_rtp_, sequence_number))
		return;

	UpdateList(sequence_number);

	sequence_num_last_received_rtp_ = sequence_number;
	timestamp_last_received_rtp_ = timestamp;
	LimitNackListSize();
}

void NackAudioModule::UpdateSamplesPerPacket(
		uint16_t sequence_number_current_received_rtp,
		uint32_t timestamp_current_received_rtp) {
	uint32_t timestamp_increase = timestamp_current_received_rtp
			- timestamp_last_received_rtp_;
	uint16_t sequence_num_increase = sequence_number_current_received_rtp
			- sequence_num_last_received_rtp_;

	samples_per_packet_ = timestamp_increase / sequence_num_increase;
}

void NackAudioModule::UpdateList(
		uint16_t sequence_number_current_received_rtp) {
	// Some of the packets which were considered late, now are considered missing.
	ChangeFromLateToMissing(sequence_number_current_received_rtp);

	if (IsNewerSequenceNumber(sequence_number_current_received_rtp,
			sequence_num_last_received_rtp_ + 1)) {
		AddToList(sequence_number_current_received_rtp);
	}
}

void NackAudioModule::ChangeFromLateToMissing(
		uint16_t sequence_number_current_received_rtp) {
	NackList::const_iterator lower_bound = nack_list_.lower_bound(
			static_cast<uint16_t>(sequence_number_current_received_rtp
					- nack_threshold_packets_));

	for (NackList::iterator it = nack_list_.begin(); it != lower_bound; ++it) {
		it->second.is_missing = true;
		LogAync(LOG_DEBUG, "NackAudioModule::ChangeFromLateToMissing( "
				"this : %p, "
				"seq_current_received : %u, "
				"seq : %u, "
				"time_to_play_ms : %lld, "
				"estimated_timestamp : %u, "
				"is_missing : %s, "
				"nack_list_.size() : %u "
				")", this, sequence_number_current_received_rtp, it->first,
				it->second.time_to_play_ms, it->second.estimated_timestamp,
				BOOL_2_STRING(it->second.is_missing), nack_list_.size());
	}
}

uint32_t NackAudioModule::EstimateTimestamp(uint16_t sequence_num) {
	uint16_t sequence_num_diff = sequence_num - sequence_num_last_received_rtp_;
	return sequence_num_diff * samples_per_packet_
			+ timestamp_last_received_rtp_;
}

void NackAudioModule::AddToList(uint16_t sequence_number_current_received_rtp) {
	// Packets with sequence numbers older than |upper_bound_missing| are
	// considered missing, and the rest are considered late.
	uint16_t upper_bound_missing = sequence_number_current_received_rtp
			- nack_threshold_packets_;

	for (uint16_t n = sequence_num_last_received_rtp_ + 1;
			IsNewerSequenceNumber(sequence_number_current_received_rtp, n);
			++n) {
		Timestamp now = clock_->CurrentTime();
		bool is_missing = IsNewerSequenceNumber(upper_bound_missing, n);
		uint32_t timestamp = EstimateTimestamp(n);
		NackElement nack_element(TimeToPlay(timestamp), timestamp, is_missing,
				now.ms());
		nack_list_.insert(nack_list_.end(), std::make_pair(n, nack_element));

		LogAync(LOG_DEBUG, "NackAudioModule::AddToList( "
				"this : %p, "
				"seq_last_received : %u, "
				"seq_current_received : %u, "
				"seq : %u, "
				"time_to_play_ms : %lld, "
				"estimated_timestamp : %u, "
				"is_missing : %s, "
				"nack_list_.size() : %u "
				")", this, sequence_num_last_received_rtp_,
				sequence_number_current_received_rtp, n,
				nack_element.time_to_play_ms, nack_element.estimated_timestamp,
				BOOL_2_STRING(nack_element.is_missing), nack_list_.size());
	}
}

void NackAudioModule::UpdateEstimatedPlayoutTime() {
	Timestamp now = clock_->CurrentTime();
	if (time_last_check_rtp_.ms() == 0) {
		time_last_check_rtp_ = now;
	}
	int64_t time_increase = now.ms() - time_last_check_rtp_.ms();

	while (!nack_list_.empty()
			&& nack_list_.begin()->second.time_to_play_ms <= time_increase) {
		NackList::iterator it = nack_list_.begin();
		LogAync(LOG_DEBUG, "NackAudioModule::UpdateEstimatedPlayoutTime( "
				"this : %p, "
				"seq : %u, "
				"time_increase : %lld, "
				"time_to_play_ms : %lld, "
				"estimated_timestamp : %u, "
				"is_missing : %s, "
				"nack_list_.size() : %u "
				")", this, it->first, time_increase, it->second.time_to_play_ms,
				it->second.estimated_timestamp,
				BOOL_2_STRING(it->second.is_missing), nack_list_.size());
		nack_list_.erase(nack_list_.begin());
	}

	for (NackList::iterator it = nack_list_.begin(); it != nack_list_.end();
			++it) {
		it->second.time_to_play_ms -= time_increase;
	}

	time_last_check_rtp_ = now;
}

//void NackAudioModule::UpdateLastDecodedPacket(uint16_t sequence_number,
//		uint32_t timestamp) {
//	if (IsNewerSequenceNumber(sequence_number, sequence_num_last_decoded_rtp_)
//			|| !any_rtp_decoded_) {
//		sequence_num_last_decoded_rtp_ = sequence_number;
//		timestamp_last_decoded_rtp_ = timestamp;
//		// Packets in the list with sequence numbers less than the
//		// sequence number of the decoded RTP should be removed from the lists.
//		// They will be discarded by the jitter buffer if they arrive.
//		nack_list_.erase(nack_list_.begin(),
//				nack_list_.upper_bound(sequence_num_last_decoded_rtp_));
//
//		// Update estimated time-to-play.
//		for (NackList::iterator it = nack_list_.begin(); it != nack_list_.end();
//				++it)
//			it->second.time_to_play_ms = TimeToPlay(
//					it->second.estimated_timestamp);
//	} else {
//		assert(sequence_number == sequence_num_last_decoded_rtp_);
//
//		// Same sequence number as before. 10 ms is elapsed, update estimations for
//		// time-to-play.
//		UpdateEstimatedPlayoutTimeBy10ms();
//
//		// Update timestamp for better estimate of time-to-play, for packets which
//		// are added to NACK list later on.
//		timestamp_last_decoded_rtp_ += sample_rate_khz_ * 10;
//	}
//	any_rtp_decoded_ = true;
//}

NackAudioModule::NackList NackAudioModule::GetNackList() const {
	return nack_list_;
}

void NackAudioModule::Reset() {
	nack_list_.clear();

	sequence_num_last_received_rtp_ = 0;
	timestamp_last_received_rtp_ = 0;
	any_rtp_received_ = false;
	sequence_num_first_received_rtp_ = 0;
	timestamp_first_received_rtp_ = 0;
	time_first_received_rtp_ = Timestamp::ms(0);
	time_last_check_rtp_ = Timestamp::ms(0);
}

void NackAudioModule::SetMaxNackListSize(size_t max_nack_list_size) {
	RTC_CHECK_GT(max_nack_list_size, 0);
	// Ugly hack to get around the problem of passing static consts by reference.
	const size_t kNackListSizeLimitLocal = NackAudioModule::kNackListSizeLimit;
	RTC_CHECK_LE(max_nack_list_size, kNackListSizeLimitLocal);

	max_nack_list_size_ = max_nack_list_size;
	LimitNackListSize();
}

void NackAudioModule::LimitNackListSize() {
	uint16_t limit = sequence_num_last_received_rtp_
			- static_cast<uint16_t>(max_nack_list_size_) - 1;

//	LogAync(LOG_DEBUG, "NackAudioModule::LimitNackListSize( "
//			"this : %p, "
//			"seq_start : %u, "
//			"seq_limit : %u, "
//			"nack_list_.size() : %u "
//			")",
//			this,
//			nack_list_.begin()->first,
//			nack_list_.upper_bound(limit)->first,
//			nack_list_.size()
//			);

	nack_list_.erase(nack_list_.begin(), nack_list_.upper_bound(limit));
}

int64_t NackAudioModule::TimeToPlay(uint32_t timestamp) const {
	Timestamp now = clock_->CurrentTime();
	int64_t time_increase = now.ms() - time_first_received_rtp_.ms();
	uint32_t timestamp_increase = timestamp - timestamp_first_received_rtp_;

	int64_t timestamp_increase_ms = timestamp_increase / sample_rate_khz_;
	int64_t delta =
			(timestamp_increase_ms > time_increase) ?
					(timestamp_increase_ms - time_increase) : 0;
	return delta + 1000;
}

// We don't erase elements with time-to-play shorter than round-trip-time.
std::vector<uint16_t> NackAudioModule::GetNackList(
		int64_t round_trip_time_ms) {
	RTC_DCHECK_GE(round_trip_time_ms, 0);
	std::vector<uint16_t> sequence_numbers;
	for (NackList::iterator it = nack_list_.begin();
			it != nack_list_.end(); ++it) {
		Timestamp now = clock_->CurrentTime();
		bool nack_on_rtt_passed = (now.ms() - it->second.sent_at_time
				>= round_trip_time_ms);
		if (it->second.is_missing
				&& it->second.time_to_play_ms > round_trip_time_ms
				&& nack_on_rtt_passed) {
			sequence_numbers.push_back(it->first);
			it->second.sent_at_time = now.ms();
		}
	}
	return sequence_numbers;
}

}  // namespace qpidnetwork
