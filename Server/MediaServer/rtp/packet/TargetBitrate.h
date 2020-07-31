/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_PACKET_TARGET_BITRATE_H_
#define RTP_PACKET_TARGET_BITRATE_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

namespace mediaserver {
namespace rtcp {
class TargetBitrate {
public:
	// TODO(sprang): This block type is just a place holder. We need to get an
	//               id assigned by IANA.
	static constexpr uint8_t kBlockType = 42;
	static const size_t kBitrateItemSizeBytes;

	struct BitrateItem {
		BitrateItem();
		BitrateItem(uint8_t spatial_layer, uint8_t temporal_layer,
				uint32_t target_bitrate_kbps);

		uint8_t spatial_layer;
		uint8_t temporal_layer;
		uint32_t target_bitrate_kbps;
	};

	TargetBitrate();
	TargetBitrate(const TargetBitrate&);
	TargetBitrate& operator=(const TargetBitrate&);
	~TargetBitrate();

	void AddTargetBitrate(uint8_t spatial_layer, uint8_t temporal_layer,
			uint32_t target_bitrate_kbps);

	const std::vector<BitrateItem>& GetTargetBitrates() const;

	void Parse(const uint8_t* block, uint16_t block_length);

	size_t BlockLength() const;

	void Create(uint8_t* buffer) const;

private:
	std::vector<BitrateItem> bitrates_;
};
}
}  // namespace mediaserver
#endif  // RTP_PACKET_TARGET_BITRATE_H_
