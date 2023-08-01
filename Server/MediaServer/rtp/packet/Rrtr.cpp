/*
 * Rrtr.cpp
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "Rrtr.h"

#include <rtp/base/byte_io.h>
#include <rtp/base/checks.h>

namespace qpidnetwork {
namespace rtcp {
// Receiver Reference Time Report Block (RFC 3611).
//
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |     BT=4      |   reserved    |       block length = 2        |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |              NTP timestamp, most significant word             |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |             NTP timestamp, least significant word             |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

void Rrtr::Parse(const uint8_t* buffer) {
	RTC_CHECK(buffer[0] == kBlockType);
	// reserved = buffer[1];
	RTC_CHECK(ByteReader<uint16_t>::ReadBigEndian(&buffer[2]) == kBlockLength);
	uint32_t seconds = ByteReader<uint32_t>::ReadBigEndian(&buffer[4]);
	uint32_t fraction = ByteReader<uint32_t>::ReadBigEndian(&buffer[8]);
	ntp_.Set(seconds, fraction);
}

void Rrtr::Create(uint8_t* buffer) const {
	const uint8_t kReserved = 0;
	buffer[0] = kBlockType;
	buffer[1] = kReserved;
	ByteWriter<uint16_t>::WriteBigEndian(&buffer[2], kBlockLength);
	ByteWriter<uint32_t>::WriteBigEndian(&buffer[4], ntp_.seconds());
	ByteWriter<uint32_t>::WriteBigEndian(&buffer[8], ntp_.fractions());
}
}
}  // namespace qpidnetwork
