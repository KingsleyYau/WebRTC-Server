/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_PACKET_REMOTE_ESTIMATE_H_
#define RTP_PACKET_REMOTE_ESTIMATE_H_

#include <memory>
#include <vector>

#include <rtp/api/transport/network_types.h>
#include <rtp/packet/app.h>

namespace mediaserver {
namespace rtcp {

class CommonHeader;
class RemoteEstimateSerializer {
public:
	virtual bool Parse(mediaserver::ArrayView<const uint8_t> src,
			NetworkStateEstimate* target) const = 0;
	virtual mediaserver::Buffer Serialize(
			const NetworkStateEstimate& src) const = 0;
	virtual ~RemoteEstimateSerializer() = default;
};

// Using a static global implementation to avoid incurring initialization
// overhead of the serializer every time RemoteEstimate is created.
const RemoteEstimateSerializer* GetRemoteEstimateSerializer();

// The RemoteEstimate packet provides network estimation results from the
// receive side. This functionality is experimental and subject to change
// without notice.
class RemoteEstimate: public App {
public:
	RemoteEstimate();
	explicit RemoteEstimate(App&& app);
	// Note, sub type must be unique among all app messages with "goog" name.
	static constexpr uint8_t kSubType = 13;
	static constexpr uint32_t kName = NameToInt("goog");
	static TimeDelta GetTimestampPeriod();

	bool ParseData();
	void SetEstimate(NetworkStateEstimate estimate);
	NetworkStateEstimate estimate() const {
		return estimate_;
	}

private:
	NetworkStateEstimate estimate_;
	const RemoteEstimateSerializer* const serializer_;
};

}  // namespace rtcp
}  // namespace mediaserver

#endif  // RTP_PACKET_REMOTE_ESTIMATE_H_
