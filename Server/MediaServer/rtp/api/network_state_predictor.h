/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_API_NETWORK_STATE_PREDICTOR_H_
#define RTP_API_NETWORK_STATE_PREDICTOR_H_

#include <memory>
#include <vector>

namespace mediaserver {

enum class BandwidthUsage {
	kBwNormal = 0, kBwUnderusing = 1, kBwOverusing = 2, kLast
};

// TODO(yinwa): work in progress. API in class NetworkStatePredictor should not
// be used by other users until this comment is removed.

// NetworkStatePredictor predict network state based on current network metrics.
// Usage:
// Setup by calling Initialize.
// For each update, call Update. Update returns network state
// prediction.
class NetworkStatePredictor {
public:
	virtual ~NetworkStatePredictor() {
	}

	// Returns current network state prediction.
	// Inputs:  send_time_ms - packet send time.
	//          arrival_time_ms - packet arrival time.
	//          network_state - computed network state.
	virtual BandwidthUsage Update(int64_t send_time_ms, int64_t arrival_time_ms,
			BandwidthUsage network_state) = 0;
};

class NetworkStatePredictorFactoryInterface {
public:
	virtual std::unique_ptr<NetworkStatePredictor>
	CreateNetworkStatePredictor() = 0;
	virtual ~NetworkStatePredictorFactoryInterface() = default;
};

}  // namespace mediaserver

#endif  // RTP_API_NETWORK_STATE_PREDICTOR_H_
