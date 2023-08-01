/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_INCLUDE_RECEIVE_STATISTICS_H_
#define RTP_INCLUDE_RECEIVE_STATISTICS_H_

#include <map>
#include <memory>
#include <vector>

#include <absl/types/optional.h>
#include <rtp/include/rtcp_statistics.h>
#include <rtp/include/rtp_rtcp_defines.h>
#include <rtp/packet/ReportBlock.h>
#include <rtp/packet/rtp_packet_received.h>

namespace qpidnetwork {

class Clock;

class ReceiveStatisticsProvider {
public:
	virtual ~ReceiveStatisticsProvider() = default;
	// Collects receive statistic in a form of rtcp report blocks.
	// Returns at most |max_blocks| report blocks.
	virtual std::vector<rtcp::ReportBlock> RtcpReportBlocks(
			size_t max_blocks) = 0;
};

class StreamStatistician {
public:
	virtual ~StreamStatistician();

	virtual RtpReceiveStats GetStats() const = 0;

	// Returns average over the stream life time.
	virtual absl::optional<int> GetFractionLostInPercent() const = 0;

	// TODO(nisse): Delete, migrate users to the above the GetStats method.
	// Gets received stream data counters (includes reset counter values).
	virtual StreamDataCounters GetReceiveStreamDataCounters() const = 0;

	virtual uint32_t BitrateReceived() const = 0;
};

class ReceiveStatistics: public ReceiveStatisticsProvider {
public:
	~ReceiveStatistics() override = default;

	static std::unique_ptr<ReceiveStatistics> Create(Clock* clock);

	virtual void OnRtpPacket(const RtpPacketReceived& packet) = 0;

	// Returns a pointer to the statistician of an ssrc.
	virtual StreamStatistician* GetStatistician(uint32_t ssrc) const = 0;

	// TODO(bugs.webrtc.org/10669): Deprecated, delete as soon as downstream
	// projects are updated. This method sets the max reordering threshold of all
	// current and future streams.
	virtual void SetMaxReorderingThreshold(int max_reordering_threshold) = 0;

	// Sets the max reordering threshold in number of packets.
	virtual void SetMaxReorderingThreshold(uint32_t ssrc,
			int max_reordering_threshold) = 0;
	// Detect retransmissions, enabling updates of the retransmitted counters. The
	// default is false.
	virtual void EnableRetransmitDetection(uint32_t ssrc, bool enable) = 0;
};

}  // namespace qpidnetwork
#endif  // RTP_INCLUDE_RECEIVE_STATISTICS_H_
