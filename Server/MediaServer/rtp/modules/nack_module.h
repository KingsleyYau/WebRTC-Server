/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_MODULES_NACK_MODULE_H_
#define RTP_MODULES_NACK_MODULE_H_

#include <stdint.h>

#include <map>
#include <set>
#include <vector>

#include <rtp/api/units/time_delta.h>
#include <rtp/base/numerics/sequence_number_util.h>
#include <rtp/base/clock.h>

namespace mediaserver {

class NackModule {
public:
	NackModule(Clock* clock);

	void Reset();

	int OnReceivedPacket(uint16_t seq_num, bool is_keyframe, std::vector<uint16_t> &nack_batch, bool &need_request_keyframe);
	int OnReceivedPacket(uint16_t seq_num, bool is_keyframe, std::vector<uint16_t> &nack_batch, bool &need_request_keyframe, bool is_recovered);

	void ClearUpTo(uint16_t seq_num);
	void UpdateRtt(int64_t rtt_ms);
	void Clear();
	void GetNackList(std::vector<uint16_t> &nack_list);

private:
	// Which fields to consider when deciding which packet to nack in
	// GetNackBatch.
	enum NackFilterOptions {
		kSeqNumOnly, kTimeOnly, kSeqNumAndTime
	};

	// This class holds the sequence number of the packet that is in the nack list
	// as well as the meta data about when it should be nacked and how many times
	// we have tried to nack this packet.
	struct NackInfo {
		NackInfo();
		NackInfo(uint16_t seq_num, uint16_t send_at_seq_num,
				int64_t created_at_time);

		uint16_t seq_num;
		uint16_t send_at_seq_num;
		int64_t created_at_time;
		int64_t sent_at_time;
		int retries;
	};

	struct BackoffSettings {
		BackoffSettings(TimeDelta min_retry, TimeDelta max_rtt, double base);
		static absl::optional<BackoffSettings> ParseFromFieldTrials();

		// Min time between nacks.
		const TimeDelta min_retry_interval;
		// Upper bound on link-delay considered for exponential backoff.
		const TimeDelta max_rtt;
		// Base for the exponential backoff.
		const double base;
	};

	void AddPacketsToNack(uint16_t seq_num_start, uint16_t seq_num_end, bool &need_request_keyframe);

	// Removes packets from the nack list until the next keyframe. Returns true
	// if packets were removed.
	bool RemovePacketsUntilKeyFrame();
	std::vector<uint16_t> GetNackBatch(NackFilterOptions options);

	// Update the reordering distribution.
	void UpdateReorderingStatistics(uint16_t seq_num);

	// Returns how many packets we have to wait in order to receive the packet
	// with probability |probabilty| or higher.
	int WaitNumberOfPackets(float probability) const;

	// TODO(philipel): Some of the variables below are consistently used on a
	// known thread (e.g. see |initialized_|). Those probably do not need
	// synchronized access.
	std::map<uint16_t, NackInfo, DescendingSeqNumComp<uint16_t>> nack_list_;
	std::set<uint16_t, DescendingSeqNumComp<uint16_t>> keyframe_list_;
	std::set<uint16_t, DescendingSeqNumComp<uint16_t>> recovered_list_;

	bool initialized_;
	int64_t rtt_ms_;
	uint16_t newest_seq_num_;

	// Only touched on the process thread.
	int64_t next_process_time_ms_;

	// Adds a delay before send nack on packet received.
	int64_t send_nack_delay_ms_;

	const absl::optional<BackoffSettings> backoff_settings_;

	Clock* const clock_;
};

}  // namespace mediaserver

#endif  // RTP_MODULES_NACK_MODULE_H_
