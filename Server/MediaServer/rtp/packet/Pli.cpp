/*
 * Pli.cpp
 *
 *  Created on: 2020/07/15
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "Pli.h"

namespace qpidnetwork {
namespace rtcp {
// RFC 4585: Feedback format.
//
// Common packet format:
//
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |V=2|P|   FMT   |       PT      |          length               |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                  SSRC of packet sender                        |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                  SSRC of media source                         |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  :            Feedback Control Information (FCI)                 :
//  :                                                               :
Pli::Pli() = default;

Pli::~Pli() = default;
//
// Picture loss indication (PLI) (RFC 4585).
// FCI: no feedback control information.
bool Pli::Parse(const CommonHeader& packet) {
	if (packet.payload_size_bytes() < kCommonFeedbackLength) {
		LogAync(LOG_WARN, "Pli::Parse( "
				"this:%p, "
				"[RTCP packet error, it is too small to be a valid PLI], "
				"packet.payload_size_bytes():%u "
				")", this, packet.payload_size_bytes());
		return false;
	}

	ParseCommonFeedback(packet.payload());
	return true;
}

size_t Pli::BlockLength() const {
	return kHeaderLength + kCommonFeedbackLength;
}

bool Pli::Create(uint8_t* packet, size_t* index, size_t max_length) const {
	if (*index + BlockLength() > max_length) {
		return false;
	}

	CreateHeader(kFeedbackMessageType, kPacketType, HeaderLength(), packet,
			index);
	CreateCommonFeedback(packet + *index);
	*index += kCommonFeedbackLength;
	return true;
}
}
} /* namespace qpidnetwork */
