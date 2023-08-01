/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include <rtp/modules/nack_module.h>

#include <algorithm>
#include <limits>

#include <rtp/api/units/timestamp.h>
#include <rtp/base/checks.h>

#include <common/LogManager.h>

namespace qpidnetwork {

namespace {
const int kMaxPacketAge = 10000;
const int kMaxNackPackets = 1000;
const int kDefaultRttMs = 100;
const int kMaxNackRetries = 10;
const int kProcessFrequency = 50;
const int kProcessIntervalMs = 1000 / kProcessFrequency;
const int kMaxReorderedPackets = 128;
const int kNumReorderingBuckets = 10;
const int kDefaultSendNackDelayMs = 0;

}  // namespace

NackModule::NackInfo::NackInfo() :
		seq_num(0), send_at_seq_num(0), sent_at_time(-1), retries(0) {
}

NackModule::NackInfo::NackInfo(uint16_t seq_num, uint16_t send_at_seq_num,
		int64_t created_at_time) :
		seq_num(seq_num), send_at_seq_num(send_at_seq_num), created_at_time(
				created_at_time), sent_at_time(-1), retries(0) {
}

NackModule::BackoffSettings::BackoffSettings(TimeDelta min_retry,
		TimeDelta max_rtt, double base) :
		min_retry_interval(min_retry), max_rtt(max_rtt), base(base) {
}

absl::optional<NackModule::BackoffSettings> NackModule::BackoffSettings::ParseFromFieldTrials() {
	// Matches magic number in RTPSender::OnReceivedNack().
//	const TimeDelta kDefaultMinRetryInterval = TimeDelta::ms(5);
//	// Upper bound on link-delay considered for exponential backoff.
//	// Selected so that cumulative delay with 1.25 base and 10 retries ends up
//	// below 3s, since above that there will be a FIR generated instead.
//	const TimeDelta kDefaultMaxRtt = TimeDelta::ms(160);
//	// Default base for exponential backoff, adds 25% RTT delay for each retry.
//	const double kDefaultBase = 1.25;
//
//	FieldTrialParameter<bool> enabled("enabled", false);
//	FieldTrialParameter<TimeDelta> min_retry("min_retry",
//			kDefaultMinRetryInterval);
//	FieldTrialParameter < TimeDelta > max_rtt("max_rtt", kDefaultMaxRtt);
//	FieldTrialParameter<double> base("base", kDefaultBase);
//	ParseFieldTrial( { &enabled, &min_retry, &max_rtt, &base },
//			field_trial::FindFullName("WebRTC-ExponentialNackBackoff"));
//
//	if (enabled) {
//		return NackModule::BackoffSettings(min_retry.Get(), max_rtt.Get(),
//				base.Get());
//	}
	return absl::nullopt;
}

NackModule::NackModule(Clock* clock) :
		clock_(clock) {
	Reset();
}

void NackModule::Reset() {
	initialized_ = false;
	rtt_ms_ = kDefaultRttMs;
	newest_seq_num_ = 0;
	send_nack_delay_ms_ = kDefaultSendNackDelayMs;
}
//NackModule::NackModule(Clock* clock, NackSender* nack_sender,
//		KeyFrameRequestSender* keyframe_request_sender) :
//		clock_(clock), nack_sender_(nack_sender), keyframe_request_sender_(
//				keyframe_request_sender), reordering_histogram_(
//				kNumReorderingBuckets, kMaxReorderedPackets), initialized_(
//				false), rtt_ms_(kDefaultRttMs), newest_seq_num_(0), next_process_time_ms_(
//				-1), send_nack_delay_ms_(GetSendNackDelay()), backoff_settings_(
//				BackoffSettings::ParseFromFieldTrials()) {
//	RTC_DCHECK(clock_); RTC_DCHECK(nack_sender_); RTC_DCHECK(keyframe_request_sender_);
//}

int NackModule::OnReceivedPacket(uint16_t seq_num, bool is_keyframe,
		std::vector<uint16_t> &nack_batch, bool &need_request_keyframe) {
	return OnReceivedPacket(seq_num, is_keyframe, nack_batch, need_request_keyframe, false);
}

int NackModule::OnReceivedPacket(uint16_t seq_num, bool is_keyframe,
		std::vector<uint16_t> &nack_batch, bool &need_request_keyframe, bool is_recovered) {
	// TODO(philipel): When the packet includes information whether it is
	//                 retransmitted or not, use that value instead. For
	//                 now set it to true, which will cause the reordering
	//                 statistics to never be updated.
	bool is_retransmitted = true;
	need_request_keyframe = false;

	if (!initialized_) {
		newest_seq_num_ = seq_num;
		if (is_keyframe) {
			keyframe_list_.insert(seq_num);
		}
		initialized_ = true;
		return 0;
	}

	// Since the |newest_seq_num_| is a packet we have actually received we know
	// that packet has never been Nacked.
	if (seq_num == newest_seq_num_) {
		return 0;
	}

	if (AheadOf(newest_seq_num_, seq_num)) {
		// An out of order packet has been received.
		auto nack_list_it = nack_list_.find(seq_num);
		int nacks_sent_for_packet = 0;
		if (nack_list_it != nack_list_.end()) {
			nacks_sent_for_packet = nack_list_it->second.retries;
			nack_list_.erase(nack_list_it);
			LogAync(LOG_DEBUG, "NackModule::OnReceivedPacket( "
					"this : %p, "
					"[Rtp Nack Ack], seq : %u "
					")", this, nack_list_it->first);
		}
		return nacks_sent_for_packet;
	}

	// Keep track of new keyframes.
	if (is_keyframe)
		keyframe_list_.insert(seq_num);

	// And remove old ones so we don't accumulate keyframes.
	auto it = keyframe_list_.lower_bound(seq_num - kMaxPacketAge);
	if (it != keyframe_list_.begin())
		keyframe_list_.erase(keyframe_list_.begin(), it);

	if (is_recovered) {
		recovered_list_.insert(seq_num);

		// Remove old ones so we don't accumulate recovered packets.
		auto it = recovered_list_.lower_bound(seq_num - kMaxPacketAge);
		if (it != recovered_list_.begin())
			recovered_list_.erase(recovered_list_.begin(), it);

		// Do not send nack for packets recovered by FEC or RTX.
		return 0;
	}

	AddPacketsToNack(newest_seq_num_ + 1, seq_num, need_request_keyframe);
	newest_seq_num_ = seq_num;

	// Are there any nacks that are waiting for this seq_num.
	nack_batch = GetNackBatch(kSeqNumAndTime);
	if (!nack_batch.empty()) {
		// This batch of NACKs is triggered externally; the initiator can
		// batch them with other feedback messages.
//		nack_sender_->SendNack(nack_batch, /*buffering_allowed=*/true);
	}

	return 0;
}

void NackModule::ClearUpTo(uint16_t seq_num) {
	nack_list_.erase(nack_list_.begin(), nack_list_.lower_bound(seq_num));
	keyframe_list_.erase(keyframe_list_.begin(),
			keyframe_list_.lower_bound(seq_num));
	recovered_list_.erase(recovered_list_.begin(),
			recovered_list_.lower_bound(seq_num));
}

void NackModule::UpdateRtt(int64_t rtt_ms) {
	rtt_ms_ = rtt_ms;
}

void NackModule::Clear() {
	nack_list_.clear();
	keyframe_list_.clear();
	recovered_list_.clear();
}

void NackModule::GetNackList(std::vector<uint16_t> &nack_list) {
	for (std::map<uint16_t, NackInfo, DescendingSeqNumComp<uint16_t>>::const_iterator itr =
			nack_list_.begin(); itr != nack_list_.end(); itr++) {
		nack_list.push_back(itr->first);
	}
}

bool NackModule::RemovePacketsUntilKeyFrame() {
	while (!keyframe_list_.empty()) {
		auto it = nack_list_.lower_bound(*keyframe_list_.begin());

		if (it != nack_list_.begin()) {
			// We have found a keyframe that actually is newer than at least one
			// packet in the nack list.
			nack_list_.erase(nack_list_.begin(), it);
			return true;
		}

		// If this keyframe is so old it does not remove any packets from the list,
		// remove it from the list of keyframes and try the next keyframe.
		keyframe_list_.erase(keyframe_list_.begin());
	}
	return false;
}

void NackModule::AddPacketsToNack(uint16_t seq_num_start,
		uint16_t seq_num_end, bool &need_request_keyframe) {
	// Remove old packets.
	auto it = nack_list_.lower_bound(seq_num_end - kMaxPacketAge);
	nack_list_.erase(nack_list_.begin(), it);

	// If the nack list is too large, remove packets from the nack list until
	// the latest first packet of a keyframe. If the list is still too large,
	// clear it and request a keyframe.
	uint16_t num_new_nacks = ForwardDiff(seq_num_start, seq_num_end);
	if (nack_list_.size() + num_new_nacks > kMaxNackPackets) {
		while (RemovePacketsUntilKeyFrame()
				&& nack_list_.size() + num_new_nacks > kMaxNackPackets) {
		}

		if (nack_list_.size() + num_new_nacks > kMaxNackPackets) {
			nack_list_.clear();
			LogAync(LOG_DEBUG, "NackModule::AddPacketsToNack( "
					"this : %p, "
					"[Clear Nack List], "
					"nack_list_.size() : %u, "
					"num_new_nacks : %u "
					")", this,
					nack_list_.size(),
					num_new_nacks);
//			RTC_LOG(LS_WARNING) << "NACK list full, clearing NACK"
//					" list and requesting keyframe.";
//			keyframe_request_sender_->RequestKeyFrame();
			need_request_keyframe = true;
			return;
		}
	}

	for (uint16_t seq_num = seq_num_start; seq_num != seq_num_end; ++seq_num) {
		// Do not send nack for packets that are already recovered by FEC or RTX
		if (recovered_list_.find(seq_num) != recovered_list_.end())
			continue;
		NackInfo nack_info(seq_num, seq_num + WaitNumberOfPackets(0.5),
				clock_->TimeInMilliseconds());
		RTC_DCHECK(nack_list_.find(seq_num) == nack_list_.end());
		nack_list_[seq_num] = nack_info;
	}
}

std::vector<uint16_t> NackModule::GetNackBatch(NackFilterOptions options) {
	bool consider_seq_num = options != kTimeOnly;
	bool consider_timestamp = options != kSeqNumOnly;
	Timestamp now = clock_->CurrentTime();
	std::vector<uint16_t> nack_batch;
	auto it = nack_list_.begin();
	while (it != nack_list_.end()) {
		TimeDelta resend_delay = TimeDelta::ms(rtt_ms_);
		if (backoff_settings_) {
			resend_delay = std::max(resend_delay,
					backoff_settings_->min_retry_interval);
			if (it->second.retries > 1) {
				TimeDelta exponential_backoff = std::min(TimeDelta::ms(rtt_ms_),
						backoff_settings_->max_rtt)
						* std::pow(backoff_settings_->base,
								it->second.retries - 1);
				resend_delay = std::max(resend_delay, exponential_backoff);
			}
		}

		bool delay_timed_out = now.ms() - it->second.created_at_time
				>= send_nack_delay_ms_;
		bool nack_on_rtt_passed = now.ms() - it->second.sent_at_time
				>= resend_delay.ms();
		bool nack_on_seq_num_passed = it->second.sent_at_time == -1
				&& AheadOrAt(newest_seq_num_, it->second.send_at_seq_num);
		if (delay_timed_out
				&& ((consider_seq_num && nack_on_seq_num_passed)
						|| (consider_timestamp && nack_on_rtt_passed))) {
			nack_batch.emplace_back(it->second.seq_num);
			++it->second.retries;
			it->second.sent_at_time = now.ms();
			if (it->second.retries >= kMaxNackRetries) {
//				RTC_LOG(LS_WARNING) << "Sequence number " << it->second.seq_num
//						<< " removed from NACK list due to max retries.";
				LogAync(LOG_DEBUG, "NackModule::GetNackBatch( "
						"this : %p, "
						"[Remove due to max retries.], seq : %u "
						")", this, it->second.seq_num);
				it = nack_list_.erase(it);
			} else {
				++it;
			}
			continue;
		}
		++it;
	}

	return nack_batch;
}

int NackModule::WaitNumberOfPackets(float probability) const {
	return 0;
}

}  // namespace webrtc
