/*
 * Fir.cpp
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "Fir.h"

namespace mediaserver {
namespace rtcp {
// RFC 4585: Feedback format.
// Common packet format:
//
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |V=2|P|   FMT   |       PT      |          length               |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                  SSRC of packet sender                        |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |             SSRC of media source (unused) = 0                 |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  :            Feedback Control Information (FCI)                 :
//  :                                                               :
// Full intra request (FIR) (RFC 5104).
// The Feedback Control Information (FCI) for the Full Intra Request
// consists of one or more FCI entries.
// FCI:
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                              SSRC                             |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  | Seq nr.       |    Reserved = 0                               |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

Fir::Fir() = default;

Fir::Fir(const Fir& fir) = default;

Fir::~Fir() = default;

bool Fir::Parse(const CommonHeader& packet) {
	RTC_CHECK_EQ(packet.type(), kPacketType); RTC_CHECK_EQ(packet.fmt(), kFeedbackMessageType);

	// The FCI field MUST contain one or more FIR entries.
	if (packet.payload_size_bytes() < kCommonFeedbackLength + kFciLength) {
		LogAync(LOG_WARNING, "Pli::Parse( "
				"this : %p, "
				"[RTCP packet error, it is too small to be a valid FIR], "
				"packet.payload_size_bytes() : %u "
				")", this, packet.payload_size_bytes());
		return false;
	}

	if ((packet.payload_size_bytes() - kCommonFeedbackLength) % kFciLength
			!= 0) {
		LogAync(LOG_WARNING, "Pli::Parse( "
				"this : %p, "
				"[RTCP packet error, it is invalid size for a valid FIR], "
				"packet.payload_size_bytes() : %u "
				")", this, packet.payload_size_bytes());
		return false;
	}

	ParseCommonFeedback(packet.payload());

	size_t number_of_fci_items = (packet.payload_size_bytes()
			- kCommonFeedbackLength) / kFciLength;
	const uint8_t* next_fci = packet.payload() + kCommonFeedbackLength;
	items_.resize(number_of_fci_items);
	for (Request& request : items_) {
		request.ssrc = ByteReader<uint32_t>::ReadBigEndian(next_fci);
		request.seq_nr = ByteReader<uint8_t>::ReadBigEndian(next_fci + 4);
		next_fci += kFciLength;
	}
	return true;
}

size_t Fir::BlockLength() const {
	return kHeaderLength + kCommonFeedbackLength + kFciLength * items_.size();
}

bool Fir::Create(uint8_t* packet, size_t* index, size_t max_length) const {
	RTC_CHECK(!items_.empty());
	if (*index + BlockLength() > max_length) {
		return false;
	}
	size_t index_end = *index + BlockLength();
	CreateHeader(kFeedbackMessageType, kPacketType, HeaderLength(), packet,
			index);
	RTC_CHECK_EQ(media_ssrc_, 0);
	CreateCommonFeedback(packet + *index);
	*index += kCommonFeedbackLength;

	constexpr uint32_t kReserved = 0;
	for (const Request& request : items_) {
		ByteWriter<uint32_t>::WriteBigEndian(packet + *index, request.ssrc);
		ByteWriter<uint8_t>::WriteBigEndian(packet + *index + 4,
				request.seq_nr);
		ByteWriter<uint32_t, 3>::WriteBigEndian(packet + *index + 5, kReserved);
		*index += kFciLength;
	} RTC_CHECK_EQ(*index, index_end);
	return true;
}
}
} /* namespace mediaserver */
