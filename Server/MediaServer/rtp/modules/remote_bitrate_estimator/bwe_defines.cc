/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include <rtp/modules/remote_bitrate_estimator/bwe_defines.h>

#include <rtp/base/field_trial.h>

namespace mediaserver {

const char kBweTypeHistogram[] = "WebRTC.BWE.Types";

namespace congestion_controller {
int GetMinBitrateBps() {
	constexpr int kMinBitrateBps = 5000;
	return kMinBitrateBps;
}

DataRate GetMinBitrate() {
	return DataRate::bps(GetMinBitrateBps());
}

}  // namespace congestion_controller

RateControlInput::RateControlInput(BandwidthUsage bw_state,
		const absl::optional<DataRate>& estimated_throughput) :
		bw_state(bw_state), estimated_throughput(estimated_throughput) {
}

RateControlInput::~RateControlInput() = default;

}  // namespace mediaserver
