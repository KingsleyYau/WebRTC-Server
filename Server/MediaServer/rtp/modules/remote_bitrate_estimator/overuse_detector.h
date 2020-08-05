/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_MODULES_REMOTE_BITRATE_ESTIMATOR_OVERUSE_DETECTOR_H_
#define RTP_MODULES_REMOTE_BITRATE_ESTIMATOR_OVERUSE_DETECTOR_H_

#include <stdint.h>

#include <rtp/api/transport/webrtc_key_value_config.h>
#include <rtp/modules/remote_bitrate_estimator/bwe_defines.h>
#include <rtp/base/constructor_magic.h>

namespace mediaserver {

bool AdaptiveThresholdExperimentIsDisabled(
		const WebRtcKeyValueConfig& key_value_config);

class OveruseDetector {
public:
	explicit OveruseDetector(const WebRtcKeyValueConfig* key_value_config);
	virtual ~OveruseDetector();
	void Reset();

	// Update the detection state based on the estimated inter-arrival time delta
	// offset. |timestamp_delta| is the delta between the last timestamp which the
	// estimated offset is based on and the last timestamp on which the last
	// offset was based on, representing the time between detector updates.
	// |num_of_deltas| is the number of deltas the offset estimate is based on.
	// Returns the state after the detection update.
	BandwidthUsage Detect(double offset, double timestamp_delta,
			int num_of_deltas, int64_t now_ms);

	// Returns the current detector state.
	BandwidthUsage State() const;

private:
	void UpdateThreshold(double modified_offset, int64_t now_ms);
	void InitializeExperiment(const WebRtcKeyValueConfig& key_value_config);

	bool in_experiment_;
	double k_up_;
	double k_down_;
	double overusing_time_threshold_;
	double threshold_;
	int64_t last_update_ms_;
	double prev_offset_;
	double time_over_using_;
	int overuse_counter_;
	BandwidthUsage hypothesis_;

	RTC_DISALLOW_COPY_AND_ASSIGN(OveruseDetector)
	;
};
}  // namespace mediaserver

#endif  // RTP_MODULES_REMOTE_BITRATE_ESTIMATOR_OVERUSE_DETECTOR_H_
