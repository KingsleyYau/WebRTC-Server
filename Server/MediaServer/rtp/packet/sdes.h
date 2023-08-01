/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_PACKET_SDES_H_
#define RTP_PACKET_SDES_H_

#include <string>
#include <vector>

#include <rtp/packet/RtcpPacket.h>

namespace qpidnetwork {
namespace rtcp {
class CommonHeader;
// Source Description (SDES) (RFC 3550).
class Sdes: public RtcpPacket {
public:
	struct Chunk {
		uint32_t ssrc;
		std::string cname;
	};
	static constexpr uint8_t kPacketType = 202;
	static constexpr size_t kMaxNumberOfChunks = 0x1f;

	Sdes();
	~Sdes() override;

	// Parse assumes header is already parsed and validated.
	bool Parse(const CommonHeader& packet);

	bool AddCName(uint32_t ssrc, std::string cname);

	const std::vector<Chunk>& chunks() const {
		return chunks_;
	}

	size_t BlockLength() const override;

	bool Create(uint8_t* packet, size_t* index, size_t max_length) const
			override;

private:
	std::vector<Chunk> chunks_;
	size_t block_length_;
};
}  // namespace rtcp
}  // namespace qpidnetwork
#endif  // RTP_PACKET_SDES_H_
