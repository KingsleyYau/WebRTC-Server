/*
 * CommonHeader.cpp
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 */

#include "CommonHeader.h"

namespace mediaserver {
namespace rtcp {
constexpr size_t CommonHeader::kHeaderSizeBytes;
//    0                   1           1       2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 0 |V=2|P|   C/F   |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 1                 |  Packet Type  |
//   ----------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 2                                 |             length            |
//   --------------------------------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Common header for all RTCP packets, 4 octets.
bool CommonHeader::Parse(const uint8_t* buffer, size_t size_bytes) {
	const uint8_t kVersion = 2;

	if (size_bytes < kHeaderSizeBytes) {
		LogAync(LOG_WARNING, "RtpPacket::Parse( "
				"this : %p, "
				"[RTCP packet error, too little data], "
				"size_bytes : %u "
				")", this, size_bytes);
		return false;
	}

	uint8_t version = buffer[0] >> 6;
	if (version != kVersion) {
		LogAync(LOG_WARNING, "RtpPacket::Parse( "
				"this : %p, "
				"[RTCP packet version error], "
				"version : %u "
				")", this, version);
		return false;
	}

	bool has_padding = (buffer[0] & 0x20) != 0;
	count_or_format_ = buffer[0] & 0x1F;
	packet_type_ = buffer[1];
	payload_size_ = ByteReader<uint16_t>::ReadBigEndian(&buffer[2]) * 4;
	payload_ = buffer + kHeaderSizeBytes;
	padding_size_ = 0;

	if (size_bytes < kHeaderSizeBytes + payload_size_) {
		LogAync(LOG_WARNING, "RtpPacket::Parse( "
				"this : %p, "
				"[RTCP packet error, buffer too small], "
				"size_bytes : %u, "
				"payload_size_ : %u "
				")", this, size_bytes, payload_size_);
		return false;
	}

	if (has_padding) {
		if (payload_size_ == 0) {
			LogAync(LOG_WARNING,
					"RtpPacket::Parse( "
							"this : %p, "
							"[RTCP packet error. Invalid RTCP header: Padding bit set but 0 payload size specified.] "
							")", this);
			return false;
		}

		padding_size_ = payload_[payload_size_ - 1];
		if (padding_size_ == 0) {
			LogAync(LOG_WARNING,
					"RtpPacket::Parse( "
							"this : %p, "
							"[RTCP packet error. Invalid RTCP header: Padding bit set but 0 padding size specified.] "
							")", this);
			return false;
		}
		if (padding_size_ > payload_size_) {
			LogAync(LOG_WARNING,
					"RtpPacket::Parse( "
							"this : %p, "
							"[RTCP packet error. Invalid RTCP header: Too many padding bytes.], "
							"padding_size_ : %u, "
							"payload_size_ : %u "
							")", this, padding_size_, payload_size_);
			return false;
		}
		payload_size_ -= padding_size_;
	}
	return true;
}
}
}  // namespace mediaserver
