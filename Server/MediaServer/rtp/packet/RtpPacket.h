/*
 * RtpPacket.h
 *
 *  Created on: 2020/07/14
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_PACKET_RTPPACKET_H_
#define RTP_PACKET_RTPPACKET_H_

#include <unistd.h>
#include <stdint.h>

#include <rtp/base/byte_io.h>
#include <rtp/api/rtp_parameters.h>
#include <rtp/include/rtp_rtcp_defines.h>
#include <rtp/include/rtp_header_extension_map.h>

#include <map>
#include <vector>
using namespace std;

namespace mediaserver {
class RtpPacket {
public:
	using ExtensionType = RTPExtensionType;
	using ExtensionManager = RtpHeaderExtensionMap;

	RtpPacket();
	RtpPacket(const ExtensionManager* extensions);
	virtual ~RtpPacket();

	void Reset();
	bool Parse(const uint8_t* buffer, size_t size);

	// Header.
	bool Marker() const {
		return marker_;
	}
	uint8_t PayloadType() const {
		return payload_type_;
	}
	uint16_t SequenceNumber() const {
		return sequence_number_;
	}
	uint32_t Timestamp() const {
		return timestamp_;
	}
	uint32_t Ssrc() const {
		return ssrc_;
	}
	std::vector<uint32_t> Csrcs() const;

	size_t headers_size() const {
		return payload_offset_;
	}

	// Payload.
	size_t payload_size() const {
		return payload_size_;
	}
	size_t padding_size() const {
		return padding_size_;
	}

	const uint8_t* data() const { return parse_data_; }

	void SetMarker(bool marker_bit);
	void SetPayloadType(uint8_t payload_type);
	void SetSequenceNumber(uint16_t seq_no);
	void SetTimestamp(uint32_t timestamp);
	void SetSsrc(uint32_t ssrc);
	// Writes csrc list. Assumes:
	// a) There is enough room left in buffer.
	// b) Extension headers, payload or padding data has not already been added.
	void SetCsrcs(mediaserver::ArrayView<const uint32_t> csrcs);

	// Header.
	bool has_padding_;
	bool has_extension_;
	uint8_t number_of_crcs_;
	bool marker_;
	uint8_t payload_type_;
	uint8_t padding_size_;
	uint16_t sequence_number_;
	uint32_t timestamp_;
	uint32_t ssrc_;
	size_t payload_offset_;  // Match header size with csrcs and extensions.
	size_t payload_size_;
	const uint8_t* parse_data_;

	// Find an extension |type|.
	// Returns view of the raw extension or empty view on failure.
	mediaserver::ArrayView<const uint8_t> FindExtension(
			ExtensionType type) const;

	template<typename Extension>
	bool HasExtension() const;

	template<typename Extension>
	bool IsExtensionReserved() const;

	template<typename Extension, typename FirstValue, typename ... Values>
	bool GetExtension(FirstValue first, Values ... values) const;

	template<typename Extension>
	absl::optional<typename Extension::value_type> GetExtension() const;

	template<typename Extension>
	ArrayView<const uint8_t> GetRawExtension() const;

	template<typename Extension, typename ... Values>
	bool SetExtension(Values ... values);

	template<typename Extension>
	bool ReserveExtension();

private:
	struct ExtensionInfo {
		explicit ExtensionInfo(uint8_t id) :
				ExtensionInfo(id, 0, 0) {
		}
		ExtensionInfo(uint8_t id, uint8_t length, uint16_t offset) :
				id(id), length(length), offset(offset) {
		}
		uint8_t id;
		uint8_t length;
		uint16_t offset;
	};

	ExtensionInfo& FindOrCreateExtensionInfo(int id);
	// Returns pointer to extension info for a given id. Returns nullptr if not
	// found.
	const ExtensionInfo* FindExtensionInfo(int id) const;

	// Extension
	std::vector<ExtensionInfo> extension_entries_;
	ExtensionManager extensions_;
	size_t extensions_size_ = 0;  // Unaligned.
};

template<typename Extension>
bool RtpPacket::HasExtension() const {
	return HasExtension(Extension::kId);
}

template<typename Extension>
bool RtpPacket::IsExtensionReserved() const {
	return IsExtensionReserved(Extension::kId);
}

template<typename Extension, typename FirstValue, typename ... Values>
bool RtpPacket::GetExtension(FirstValue first, Values ... values) const {
	auto raw = FindExtension(Extension::kId);
	if (raw.empty())
		return false;
	return Extension::Parse(raw, first, values...);
}

template<typename Extension>
absl::optional<typename Extension::value_type> RtpPacket::GetExtension() const {
	absl::optional<typename Extension::value_type> result;
	auto raw = FindExtension(Extension::kId);
	if (raw.empty() || !Extension::Parse(raw, &result.emplace()))
		result = absl::nullopt;
	return result;
}

template<typename Extension>
mediaserver::ArrayView<const uint8_t> RtpPacket::GetRawExtension() const {
	return FindExtension(Extension::kId);
}

template<typename Extension, typename ... Values>
bool RtpPacket::SetExtension(Values ... values) {
	const size_t value_size = Extension::ValueSize(values...);
	auto buffer = AllocateExtension(Extension::kId, value_size);
	if (buffer.empty())
		return false;
	return Extension::Write(buffer, values...);
}

template<typename Extension>
bool RtpPacket::ReserveExtension() {
	auto buffer = AllocateExtension(Extension::kId, Extension::kValueSizeBytes);
	if (buffer.empty())
		return false;
	memset(buffer.data(), 0, Extension::kValueSizeBytes);
	return true;
}

} /* namespace mediaserver */

#endif /* RTP_PACKET_RTPPACKET_H_ */
