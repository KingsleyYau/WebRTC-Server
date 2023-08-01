/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_MODULES_CONGESTION_CONTROLLER_GOOG_CC_LINK_CAPACITY_ESTIMATOR_H_
#define RTP_MODULES_CONGESTION_CONTROLLER_GOOG_CC_LINK_CAPACITY_ESTIMATOR_H_

#include <absl/types/optional.h>
#include <rtp/api/units/data_rate.h>

namespace qpidnetwork {
class LinkCapacityEstimator {
public:
	LinkCapacityEstimator();
	DataRate UpperBound() const;
	DataRate LowerBound() const;
	void Reset();
	void OnOveruseDetected(DataRate acknowledged_rate);
	void OnProbeRate(DataRate probe_rate);
	bool has_estimate() const;
	DataRate estimate() const;

private:
	friend class GoogCcStatePrinter;
	void Update(DataRate capacity_sample, double alpha);

	double deviation_estimate_kbps() const;
	absl::optional<double> estimate_kbps_;
	double deviation_kbps_ = 0.4;
};
}  // namespace qpidnetwork

#endif  // RTP_MODULES_CONGESTION_CONTROLLER_GOOG_CC_LINK_CAPACITY_ESTIMATOR_H_
