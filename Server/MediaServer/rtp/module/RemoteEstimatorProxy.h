/*
 * RemoteBitrateEstimator.h
 *
 *  Created on: 2020/07/17
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_MODULE_REMOTEBITRATEESTIMATOR_H_
#define RTP_MODULE_REMOTEBITRATEESTIMATOR_H_

#include <stddef.h>
#include <stdint.h>

#include <rtp/base/Check.h>
#include <rtp/base/numerics/sequence_number_util.h>

#include <rtp/packet/RtpPacketImp.h>
#include <rtp/packet/TransportFeedback.h>

#include <absl/types/optional.h>

#include <vector>
#include <map>

namespace mediaserver {

class RemoteEstimatorProxy {
public:
	RemoteEstimatorProxy();
	virtual ~RemoteEstimatorProxy();

	void Reset();
	void RecvRtpPacket(int64_t arrival_time_ms, const RtpPacketImp* pkt);
	bool SendPeriodicFeedbacks(uint8_t* packet, size_t* index, size_t max_length);

private:
	int64_t BuildFeedbackPacket(uint8_t feedback_packet_count,
			uint32_t media_ssrc, int64_t base_sequence_number,
			std::map<int64_t, int64_t>::const_iterator begin_iterator,
			std::map<int64_t, int64_t>::const_iterator end_iterator,
			TransportFeedback* feedback_packet);

	static const int kMaxNumberOfPackets;

	uint32_t media_ssrc_;
	uint8_t feedback_packet_count_;

    // Map seq -> time.
    std::map<int64_t, int64_t> packet_arrival_times_;
	int64_t window_seq_;
	int64_t last_send_time;
};

} /* namespace mediaserver */

#endif /* RTP_MODULE_REMOTEBITRATEESTIMATOR_H_ */
