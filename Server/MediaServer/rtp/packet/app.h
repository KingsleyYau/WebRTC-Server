/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_PACKET_RTCP_PACKET_APP_H_
#define RTP_PACKET_RTCP_PACKET_APP_H_

#include <stddef.h>
#include <stdint.h>

#include <rtp/packet/RtcpPacket.h>
#include <rtp/base/buffer.h>

namespace mediaserver {
namespace rtcp {
class CommonHeader;

class App: public RtcpPacket {
public:
	static constexpr uint8_t kPacketType = 204;
	App();
	App(App&&) = default;
	~App() override;

	// Parse assumes header is already parsed and validated.
	bool Parse(const CommonHeader& packet);

	void SetSubType(uint8_t subtype);
	void SetName(uint32_t name) {
		name_ = name;
	}
	void SetData(const uint8_t* data, size_t data_length);

	uint8_t sub_type() const {
		return sub_type_;
	}
	uint32_t name() const {
		return name_;
	}
	size_t data_size() const {
		return data_.size();
	}
	const uint8_t* data() const {
		return data_.data();
	}

	size_t BlockLength() const override;

	bool Create(uint8_t* packet, size_t* index, size_t max_length) const override;

	static inline constexpr uint32_t NameToInt(const char name[5]) {
		return static_cast<uint32_t>(name[0]) << 24
				| static_cast<uint32_t>(name[1]) << 16
				| static_cast<uint32_t>(name[2]) << 8
				| static_cast<uint32_t>(name[3]);
	}

private:
	static constexpr size_t kAppBaseLength = 8;  // Ssrc and Name.
	static constexpr size_t kMaxDataSize = 0xffff * 4 - kAppBaseLength;

	uint8_t sub_type_;
	uint32_t name_;
	mediaserver::Buffer data_;
};

}  // namespace rtcp
}  // namespace mediaserver
#endif  // RTP_PACKET_RTCP_PACKET_APP_H_
