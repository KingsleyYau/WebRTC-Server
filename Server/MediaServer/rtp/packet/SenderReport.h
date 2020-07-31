/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_PACKET_SENDER_REPORT_H_
#define RTP_PACKET_SENDER_REPORT_H_

#include <vector>

#include <rtp/base/ntp_time.h>

#include <rtp/packet/RtcpPacket.h>
#include <rtp/packet/ReportBlock.h>

namespace mediaserver {
namespace rtcp {
class CommonHeader;

class SenderReport: public RtcpPacket {
public:
	static constexpr uint8_t kPacketType = 200;
	static constexpr size_t kMaxNumberOfReportBlocks = 0x1f;

	SenderReport();
	SenderReport(const SenderReport&);
	SenderReport(SenderReport&&);
	SenderReport& operator=(const SenderReport&);
	SenderReport& operator=(SenderReport&&);
	~SenderReport() override;

	// Parse assumes header is already parsed and validated.
	bool Parse(const CommonHeader& packet);

	void SetNtp(NtpTime ntp) {
		ntp_ = ntp;
	}
	void SetRtpTimestamp(uint32_t rtp_timestamp) {
		rtp_timestamp_ = rtp_timestamp;
	}
	void SetPacketCount(uint32_t packet_count) {
		sender_packet_count_ = packet_count;
	}
	void SetOctetCount(uint32_t octet_count) {
		sender_octet_count_ = octet_count;
	}
	bool AddReportBlock(const ReportBlock& block);
	bool SetReportBlocks(std::vector<ReportBlock> blocks);
	void ClearReportBlocks() {
		report_blocks_.clear();
	}

	NtpTime ntp() const {
		return ntp_;
	}
	uint32_t rtp_timestamp() const {
		return rtp_timestamp_;
	}
	uint32_t sender_packet_count() const {
		return sender_packet_count_;
	}
	uint32_t sender_octet_count() const {
		return sender_octet_count_;
	}

	const std::vector<ReportBlock>& report_blocks() const {
		return report_blocks_;
	}

	size_t BlockLength() const override;

	bool Create(uint8_t* packet, size_t* index, size_t max_length) const
			override;

private:
	static constexpr size_t kSenderBaseLength = 24;

	NtpTime ntp_;
	uint32_t rtp_timestamp_;
	uint32_t sender_packet_count_;
	uint32_t sender_octet_count_;
	std::vector<ReportBlock> report_blocks_;
};
} // namespace rtcp
} // namespace mediaserver
#endif  // RTP_PACKET_SENDER_REPORT_H_
