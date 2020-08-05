/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_MODULES_REMOTE_BITRATE_ESTIMATOR_BWE_DEFINES_H_
#define RTP_MODULES_REMOTE_BITRATE_ESTIMATOR_BWE_DEFINES_H_

#include <stdint.h>

#include <absl/types/optional.h>
#include <rtp/api/network_state_predictor.h>
#include <rtp/api/units/data_rate.h>

#define BWE_MAX(a, b) ((a) > (b) ? (a) : (b))
#define BWE_MIN(a, b) ((a) < (b) ? (a) : (b))

namespace mediaserver {

namespace congestion_controller {
int GetMinBitrateBps();
DataRate GetMinBitrate();
}  // namespace congestion_controller

static const int64_t kBitrateWindowMs = 1000;

extern const char kBweTypeHistogram[];

enum BweNames {
	kReceiverNoExtension = 0,
	kReceiverTOffset = 1,
	kReceiverAbsSendTime = 2,
	kSendSideTransportSeqNum = 3,
	kBweNamesMax = 4
};

enum RateControlState {
	kRcHold, kRcIncrease, kRcDecrease
};

struct RateControlInput {
	RateControlInput(BandwidthUsage bw_state,
			const absl::optional<DataRate>& estimated_throughput);
	~RateControlInput();

	BandwidthUsage bw_state;
	absl::optional<DataRate> estimated_throughput;
};
}  // namespace mediaserver

#endif  // RTP_MODULES_REMOTE_BITRATE_ESTIMATOR_BWE_DEFINES_H_
