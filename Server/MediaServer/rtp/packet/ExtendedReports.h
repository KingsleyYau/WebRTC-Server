/*
 * ExtendedReports.h
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_PACKET_EXTENDED_REPORTS_H_
#define RTP_PACKET_EXTENDED_REPORTS_H_

#include <vector>

#include <absl/types/optional.h>

#include <rtp/packet/RtcpPacket.h>
#include <rtp/packet/Dlrr.h>
#include <rtp/packet/Rrtr.h>
#include <rtp/packet/TargetBitrate.h>

namespace mediaserver {
namespace rtcp {
class CommonHeader;

// From RFC 3611: RTP Control Protocol Extended Reports (RTCP XR).
class ExtendedReports: public RtcpPacket {
public:
	static constexpr uint8_t kPacketType = 207;
	static constexpr size_t kMaxNumberOfDlrrItems = 50;

	ExtendedReports();
	ExtendedReports(const ExtendedReports& xr);
	~ExtendedReports() override;

	// Parse assumes header is already parsed and validated.
	bool Parse(const CommonHeader& packet);

	void SetRrtr(const Rrtr& rrtr);
	bool AddDlrrItem(const ReceiveTimeInfo& time_info);
	void SetTargetBitrate(const TargetBitrate& target_bitrate);

	const absl::optional<Rrtr>& rrtr() const {
		return rrtr_block_;
	}
	const Dlrr& dlrr() const {
		return dlrr_block_;
	}
	const absl::optional<TargetBitrate>& target_bitrate() const {
		return target_bitrate_;
	}

	size_t BlockLength() const override;

	bool Create(uint8_t* packet, size_t* index, size_t max_length) const
			override;

private:
	static constexpr size_t kXrBaseLength = 4;

	size_t RrtrLength() const {
		return rrtr_block_ ? Rrtr::kLength : 0;
	}
	size_t DlrrLength() const {
		return dlrr_block_.BlockLength();
	}
	size_t TargetBitrateLength() const;

	void ParseRrtrBlock(const uint8_t* block, uint16_t block_length);
	void ParseDlrrBlock(const uint8_t* block, uint16_t block_length);
	void ParseVoipMetricBlock(const uint8_t* block, uint16_t block_length);
	void ParseTargetBitrateBlock(const uint8_t* block, uint16_t block_length);

	absl::optional<Rrtr> rrtr_block_;
	Dlrr dlrr_block_;  // Dlrr without items treated same as no dlrr block.
	absl::optional<TargetBitrate> target_bitrate_;
};
}
}  // namespace mediaserver
#endif  // RTP_PACKET_EXTENDED_REPORTS_H_
