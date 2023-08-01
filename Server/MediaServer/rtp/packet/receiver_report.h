/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_PACKET_RECEIVER_REPORT_H_
#define RTP_PACKET_RECEIVER_REPORT_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include <rtp/packet/RtcpPacket.h>
#include <rtp/packet/ReportBlock.h>

namespace qpidnetwork {
namespace rtcp {
class CommonHeader;

class ReceiverReport: public RtcpPacket {
public:
	static constexpr uint8_t kPacketType = 201;
	static constexpr size_t kMaxNumberOfReportBlocks = 0x1f;

	ReceiverReport();
	ReceiverReport(const ReceiverReport&);
	~ReceiverReport() override;

	// Parse assumes header is already parsed and validated.
	bool Parse(const CommonHeader& packet);

	bool AddReportBlock(const ReportBlock& block);
	bool SetReportBlocks(std::vector<ReportBlock> blocks);

	const std::vector<ReportBlock>& report_blocks() const {
		return report_blocks_;
	}

	size_t BlockLength() const override;

	bool Create(uint8_t* packet, size_t* index, size_t max_length) const override;

private:
	static const size_t kRrBaseLength = 4;

	std::vector<ReportBlock> report_blocks_;
};

}  // namespace rtcp
}  // namespace qpidnetwork
#endif  // RTP_PACKET_RECEIVER_REPORT_H_
