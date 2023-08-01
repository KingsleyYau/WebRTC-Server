/*
 * SenderReport.cpp
 *
 *  Created on: 2020/07/15
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "SenderReport.h"

#include <utility>

#include <rtp/base/byte_io.h>
#include <rtp/base/checks.h>
#include <rtp/packet/CommonHeader.h>

namespace qpidnetwork {
namespace rtcp {
constexpr uint8_t SenderReport::kPacketType;
constexpr size_t SenderReport::kMaxNumberOfReportBlocks;
constexpr size_t SenderReport::kSenderBaseLength;
//    Sender report (SR) (RFC 3550).
//     0                   1                   2                   3
//     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |V=2|P|    RC   |   PT=SR=200   |             length            |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  0 |                         SSRC of sender                        |
//    +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//  4 |              NTP timestamp, most significant word             |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  8 |             NTP timestamp, least significant word             |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 12 |                         RTP timestamp                         |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 16 |                     sender's packet count                     |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 20 |                      sender's octet count                     |
// 24 +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+

SenderReport::SenderReport() :
		rtp_timestamp_(0), sender_packet_count_(0), sender_octet_count_(0) {
}

SenderReport::SenderReport(const SenderReport&) = default;
SenderReport::SenderReport(SenderReport&&) = default;
SenderReport& SenderReport::operator=(const SenderReport&) = default;
SenderReport& SenderReport::operator=(SenderReport&&) = default;
SenderReport::~SenderReport() = default;

bool SenderReport::Parse(const CommonHeader& packet) {
	RTC_CHECK_EQ(packet.type(), kPacketType);

	const uint8_t report_block_count = packet.count();
	if (packet.payload_size_bytes()
			< kSenderBaseLength + report_block_count * ReportBlock::kLength) {
//		RTC_LOG(LS_WARNING) << "Packet is too small to contain all the data.";
		return false;
	}
	// Read SenderReport header.
	const uint8_t* const payload = packet.payload();
	sender_ssrc_ = (ByteReader<uint32_t>::ReadBigEndian(&payload[0]));
	uint32_t secs = ByteReader<uint32_t>::ReadBigEndian(&payload[4]);
	uint32_t frac = ByteReader<uint32_t>::ReadBigEndian(&payload[8]);
	ntp_.Set(secs, frac);
	rtp_timestamp_ = ByteReader<uint32_t>::ReadBigEndian(&payload[12]);
	sender_packet_count_ = ByteReader<uint32_t>::ReadBigEndian(&payload[16]);
	sender_octet_count_ = ByteReader<uint32_t>::ReadBigEndian(&payload[20]);
	report_blocks_.resize(report_block_count);
	const uint8_t* next_block = payload + kSenderBaseLength;
	for (ReportBlock& block : report_blocks_) {
		bool block_parsed = block.Parse(next_block, ReportBlock::kLength);
		RTC_CHECK(block_parsed);
		next_block += ReportBlock::kLength;
	}
	// Double check we didn't read beyond provided buffer.
	RTC_CHECK_LE(next_block - payload,
			static_cast<ptrdiff_t>(packet.payload_size_bytes()));
	return true;
}

size_t SenderReport::BlockLength() const {
	return kHeaderLength + kSenderBaseLength
			+ report_blocks_.size() * ReportBlock::kLength;
}

bool SenderReport::Create(uint8_t* packet, size_t* index,
		size_t max_length) const {
	if (*index + BlockLength() > max_length) {
//    if (!OnBufferFull(packet, index, callback))
		return false;
	}
	const size_t index_end = *index + BlockLength();

	CreateHeader(report_blocks_.size(), kPacketType, HeaderLength(), packet,
			index);
	// Write SenderReport header.
	ByteWriter<uint32_t>::WriteBigEndian(&packet[*index + 0], sender_ssrc_);
	ByteWriter<uint32_t>::WriteBigEndian(&packet[*index + 4], ntp_.seconds());
	ByteWriter<uint32_t>::WriteBigEndian(&packet[*index + 8], ntp_.fractions());
	ByteWriter<uint32_t>::WriteBigEndian(&packet[*index + 12], rtp_timestamp_);
	ByteWriter<uint32_t>::WriteBigEndian(&packet[*index + 16],
			sender_packet_count_);
	ByteWriter<uint32_t>::WriteBigEndian(&packet[*index + 20],
			sender_octet_count_);
	*index += kSenderBaseLength;
	// Write report blocks.
	for (const ReportBlock& block : report_blocks_) {
		block.Create(packet + *index);
		*index += ReportBlock::kLength;
	}
	// Ensure bytes written match expected.
	RTC_CHECK_EQ(*index, index_end);
	return true;
}

bool SenderReport::AddReportBlock(const ReportBlock& block) {
	if (report_blocks_.size() >= kMaxNumberOfReportBlocks) {
//    RTC_LOG(LS_WARNING) << "Max report blocks reached.";
		return false;
	}
	report_blocks_.push_back(block);
	return true;
}

bool SenderReport::SetReportBlocks(std::vector<ReportBlock> blocks) {
	if (blocks.size() > kMaxNumberOfReportBlocks) {
//    RTC_LOG(LS_WARNING) << "Too many report blocks (" << blocks.size()
//                        << ") for sender report.";
		return false;
	}
	report_blocks_ = std::move(blocks);
	return true;
}
} // namesapce rtcp
} // namespace qpidnetwork
