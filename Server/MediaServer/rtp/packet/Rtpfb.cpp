/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */
#include "Rtpfb.h"

namespace mediaserver {
namespace rtcp {
// RFC 4585, Section 6.1: Feedback format.
//
// Common packet format:
//
//    0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |V=2|P|   FMT   |       PT      |          length               |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 0 |                  SSRC of packet sender                        |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 4 |                  SSRC of media source                         |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   :            Feedback Control Information (FCI)                 :
//   :                                                               :

void Rtpfb::ParseCommonFeedback(const uint8_t* payload) {
	sender_ssrc_ = (ByteReader<uint32_t>::ReadBigEndian(&payload[0]));
	media_ssrc_ = (ByteReader<uint32_t>::ReadBigEndian(&payload[4]));
}

void Rtpfb::CreateCommonFeedback(uint8_t* payload) const {
	ByteWriter<uint32_t>::WriteBigEndian(&payload[0], sender_ssrc_);
	ByteWriter<uint32_t>::WriteBigEndian(&payload[4], media_ssrc_);
}
}
} /* namespace mediaserver */
