/*
 * Remb.cpp
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "Remb.h"

namespace qpidnetwork {
namespace rtcp {
// Receiver Estimated Max Bitrate (REMB) (draft-alvestrand-rmcat-remb).
//
//     0                   1                   2                   3
//     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |V=2|P| FMT=15  |   PT=206      |             length            |
//    +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//  0 |                  SSRC of packet sender                        |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  4 |                       Unused = 0                              |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  8 |  Unique identifier 'R' 'E' 'M' 'B'                            |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 12 |  Num SSRC     | BR Exp    |  BR Mantissa                      |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 16 |   SSRC feedback                                               |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    :  ...                                                          :

Remb::Remb() :
		bitrate_bps_(0) {
}

Remb::Remb(const Remb& rhs) = default;

Remb::~Remb() = default;

bool Remb::Parse(const CommonHeader& packet) {
	RTC_CHECK(packet.type() == kPacketType);
	RTC_CHECK_EQ(packet.fmt(), Psfb::kAfbMessageType);

	if (packet.payload_size_bytes() < 16) {
		LogAync(LOG_WARN, "Pli::Parse( "
				"this:%p, "
				"[RTCP packet error, it is too small to be a valid Remb], "
				"packet.payload_size_bytes():%u "
				")", this, packet.payload_size_bytes());
		return false;
	}
	const uint8_t* const payload = packet.payload();
	if (kUniqueIdentifier != ByteReader<uint32_t>::ReadBigEndian(&payload[8])) {
		return false;
	}
	uint8_t number_of_ssrcs = payload[12];
	if (packet.payload_size_bytes()
			!= kCommonFeedbackLength + (2 + number_of_ssrcs) * 4) {
		LogAync(LOG_WARN, "Pli::Parse( "
				"this:%p, "
				"[RTCP packet error, payload size does not match], "
				"packet.payload_size_bytes():%u, "
				"number_of_ssrcs:%u "
				")", this, packet.payload_size_bytes(), number_of_ssrcs);
		return false;
	}

	ParseCommonFeedback(payload);
	uint8_t exponenta = payload[13] >> 2;
	uint64_t mantissa = (static_cast<uint32_t>(payload[13] & 0x03) << 16)
			| ByteReader<uint16_t>::ReadBigEndian(&payload[14]);
	bitrate_bps_ = (mantissa << exponenta);
	bool shift_overflow = (bitrate_bps_ >> exponenta) != mantissa;
	if (shift_overflow) {
		LogAync(LOG_WARN, "Pli::Parse( "
				"this:%p, "
				"[RTCP packet error, invalid remb bitrate value], "
				"mantissa:%u, "
				"exponenta:%u "
				")", this, mantissa, static_cast<int>(exponenta));
		return false;
	}

	const uint8_t* next_ssrc = payload + 16;
	ssrcs_.clear();
	ssrcs_.reserve(number_of_ssrcs);
	for (uint8_t i = 0; i < number_of_ssrcs; ++i) {
		ssrcs_.push_back(ByteReader<uint32_t>::ReadBigEndian(next_ssrc));
		next_ssrc += sizeof(uint32_t);
	}

	return true;
}

bool Remb::SetSsrcs(std::vector<uint32_t> ssrcs) {
	if (ssrcs.size() > kMaxNumberOfSsrcs) {
		LogAync(LOG_WARN, "Pli::Parse( "
				"this:%p, "
				"[RTCP packet error, Not enough space for all given SSRCs.] "
				")", this);
		return false;
	}
	ssrcs_ = std::move(ssrcs);
	return true;
}

size_t Remb::BlockLength() const {
	return kHeaderLength + kCommonFeedbackLength + (2 + ssrcs_.size()) * 4;
}

bool Remb::Create(uint8_t* packet, size_t* index, size_t max_length) const {
	if (*index + BlockLength() > max_length) {
		return false;
	}
	size_t index_end = *index + BlockLength();
	CreateHeader(Psfb::kAfbMessageType, kPacketType, HeaderLength(), packet,
			index);
	RTC_CHECK_EQ(0, Psfb::media_ssrc_);
	CreateCommonFeedback(packet + *index);
	*index += kCommonFeedbackLength;

	ByteWriter<uint32_t>::WriteBigEndian(packet + *index, kUniqueIdentifier);
	*index += sizeof(uint32_t);
	const uint32_t kMaxMantissa = 0x3ffff;  // 18 bits.
	uint64_t mantissa = bitrate_bps_;
	uint8_t exponenta = 0;
	while (mantissa > kMaxMantissa) {
		mantissa >>= 1;
		++exponenta;
	}
	packet[(*index)++] = static_cast<uint8_t>(ssrcs_.size());
	packet[(*index)++] = (exponenta << 2) | (mantissa >> 16);
	ByteWriter<uint16_t>::WriteBigEndian(packet + *index, mantissa & 0xffff);
	*index += sizeof(uint16_t);

	for (uint32_t ssrc : ssrcs_) {
		ByteWriter<uint32_t>::WriteBigEndian(packet + *index, ssrc);
		*index += sizeof(uint32_t);
	}
	RTC_CHECK_EQ(index_end, *index);
	return true;
}
}
} /* namespace qpidnetwork */
