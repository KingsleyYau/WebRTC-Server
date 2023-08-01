/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_INCLUDE_RTCP_STATISTICS_H_
#define RTP_INCLUDE_RTCP_STATISTICS_H_

#include <stdint.h>

#include <absl/strings/string_view.h>

namespace qpidnetwork {

// Statistics for an RTCP channel
struct RtcpStatistics {
	uint8_t fraction_lost = 0;
	int32_t packets_lost = 0;  // Defined as a 24 bit signed integer in RTCP
	uint32_t extended_highest_sequence_number = 0;
	uint32_t jitter = 0;
};

class RtcpStatisticsCallback {
public:
	virtual ~RtcpStatisticsCallback() {
	}

	virtual void StatisticsUpdated(const RtcpStatistics& statistics,
			uint32_t ssrc) = 0;
};

// Statistics for RTCP packet types.
struct RtcpPacketTypeCounter {
	RtcpPacketTypeCounter() :
			first_packet_time_ms(-1), nack_packets(0), fir_packets(0), pli_packets(
					0), nack_requests(0), unique_nack_requests(0) {
	}

	void Add(const RtcpPacketTypeCounter& other) {
		nack_packets += other.nack_packets;
		fir_packets += other.fir_packets;
		pli_packets += other.pli_packets;
		nack_requests += other.nack_requests;
		unique_nack_requests += other.unique_nack_requests;
		if (other.first_packet_time_ms != -1
				&& (other.first_packet_time_ms < first_packet_time_ms
						|| first_packet_time_ms == -1)) {
			// Use oldest time.
			first_packet_time_ms = other.first_packet_time_ms;
		}
	}

	void Subtract(const RtcpPacketTypeCounter& other) {
		nack_packets -= other.nack_packets;
		fir_packets -= other.fir_packets;
		pli_packets -= other.pli_packets;
		nack_requests -= other.nack_requests;
		unique_nack_requests -= other.unique_nack_requests;
		if (other.first_packet_time_ms != -1
				&& (other.first_packet_time_ms > first_packet_time_ms
						|| first_packet_time_ms == -1)) {
			// Use youngest time.
			first_packet_time_ms = other.first_packet_time_ms;
		}
	}

	int64_t TimeSinceFirstPacketInMs(int64_t now_ms) const {
		return (first_packet_time_ms == -1) ?
				-1 : (now_ms - first_packet_time_ms);
	}

	int UniqueNackRequestsInPercent() const {
		if (nack_requests == 0) {
			return 0;
		}
		return static_cast<int>((unique_nack_requests * 100.0f / nack_requests)
				+ 0.5f);
	}

	int64_t first_packet_time_ms;   // Time when first packet is sent/received.
	uint32_t nack_packets;          // Number of RTCP NACK packets.
	uint32_t fir_packets;           // Number of RTCP FIR packets.
	uint32_t pli_packets;           // Number of RTCP PLI packets.
	uint32_t nack_requests;         // Number of NACKed RTP packets.
	uint32_t unique_nack_requests;  // Number of unique NACKed RTP packets.
};

class RtcpPacketTypeCounterObserver {
public:
	virtual ~RtcpPacketTypeCounterObserver() {
	}
	virtual void RtcpPacketTypesCounterUpdated(uint32_t ssrc,
			const RtcpPacketTypeCounter& packet_counter) = 0;
};

// Invoked for each cname passed in RTCP SDES blocks.
class RtcpCnameCallback {
public:
	virtual ~RtcpCnameCallback() = default;

	virtual void OnCname(uint32_t ssrc, absl::string_view cname) = 0;
};

}  // namespace qpidnetwork
#endif  // RTP_INCLUDE_RTCP_STATISTICS_H_
