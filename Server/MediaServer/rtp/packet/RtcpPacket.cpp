/*
 * RtcpPacket.cpp
 *
 *  Created on: 2020/07/15
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "RtcpPacket.h"

namespace mediaserver {
namespace rtcp {
size_t RtcpPacket::HeaderLength() const {
	size_t length_in_bytes = BlockLength();
	RTC_CHECK_GT(length_in_bytes, 0); RTC_CHECK_EQ(length_in_bytes % 4, 0);
	// Length in 32-bit words without common header.
	return (length_in_bytes - kHeaderLength) / 4;
}

// From RFC 3550, RTP: A Transport Protocol for Real-Time Applications.
//
// RTP header format.
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |V=2|P| RC/FMT  |      PT       |             length            |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RtcpPacket::CreateHeader(size_t count_or_format, // Depends on packet type.
		uint8_t packet_type, size_t length, uint8_t* buffer, size_t* pos) {
	CreateHeader(count_or_format, packet_type, length, /*padding=*/false,
			buffer, pos);
}

void RtcpPacket::CreateHeader(
		size_t count_or_format,  // Depends on packet type.
		uint8_t packet_type, size_t length, bool padding, uint8_t* buffer,
		size_t* pos) {
	RTC_CHECK_LE(length, 0xffffU); RTC_CHECK_LE(count_or_format, 0x1f);
	constexpr uint8_t kVersionBits = 2 << 6;
	uint8_t padding_bit = padding ? 1 << 5 : 0;
	buffer[*pos + 0] = kVersionBits | padding_bit
			| static_cast<uint8_t>(count_or_format);
	buffer[*pos + 1] = packet_type;
	buffer[*pos + 2] = (length >> 8) & 0xff;
	buffer[*pos + 3] = length & 0xff;
	*pos += kHeaderLength;
}
}
} /* namespace mediaserver */
