/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_PACKET_RTPPACKET_H_
#define RTP_PACKET_RTPPACKET_H_

#include <unistd.h>
#include <stdint.h>

#include <rtp/base/byte_io.h>
#include <rtp/api/rtp_parameters.h>
#include <rtp/include/rtp_rtcp_defines.h>
#include <rtp/include/rtp_header_extension_map.h>
#include <rtp/base/copy_on_write_buffer.h>

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
	RtpPacket(const ExtensionManager* extensions, size_t capacity);
	virtual ~RtpPacket();

	bool Parse(const uint8_t* buffer, size_t buffer_size);

	// Maps extensions id to their types.
	void IdentifyExtensions(const ExtensionManager& extensions);

	/////////////////////////////////////////////////////////////////////////////////
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
	/////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////
	// Payload.
	size_t payload_size() const {
		return payload_size_;
	}
	size_t padding_size() const {
		return padding_size_;
	}
	ArrayView<const uint8_t> payload() const {
		return MakeArrayView(data() + payload_offset_, payload_size_);
	}
	/////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////
	// Buffer.
	mediaserver::CopyOnWriteBuffer Buffer() const {
		return buffer_;
	}
	size_t capacity() const {
		return buffer_.capacity();
	}
	size_t size() const {
		return payload_offset_ + payload_size_ + padding_size_;
	}
	const uint8_t* data() const {
		return parse_data_;
	}
//	const uint8_t* data() const {
//		return buffer_.cdata();
//	}
	size_t FreeCapacity() const {
		return capacity() - size();
	}
	size_t MaxPayloadSize() const {
		return capacity() - headers_size();
	}
	/////////////////////////////////////////////////////////////////////////////////

	// Reset fields and buffer.
	void Clear();

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

	// Removes extension of given |type|, returns false is extension was not
	// registered in packet's extension map or not present in the packet. Only
	// extension that should be removed must be registered, other extensions may
	// not be registered and will be preserved as is.
	bool RemoveExtension(ExtensionType type);

	template<typename Extension>
	bool HasExtension() const;
	bool HasExtension(ExtensionType type) const;

	template<typename Extension>
	bool IsExtensionReserved() const;
	bool IsExtensionReserved(ExtensionType type) const;

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

	// Find or allocate an extension |type|. Returns view of size |length|
	// to write raw extension to or an empty view on failure.
	mediaserver::ArrayView<uint8_t> AllocateExtension(ExtensionType type,
			size_t length);

	// Find an extension |type|.
	// Returns view of the raw extension or empty view on failure.
	mediaserver::ArrayView<const uint8_t> FindExtension(
			ExtensionType type) const;

	// Reserve size_bytes for payload. Returns nullptr on failure.
	uint8_t* SetPayloadSize(size_t size_bytes);
	// Same as SetPayloadSize but doesn't guarantee to keep current payload.
	uint8_t* AllocatePayload(size_t size_bytes);

	bool SetPadding(size_t padding_size);

	// Returns debug string of RTP packet (without detailed extension info).
	std::string ToString() const;

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

	// Helper function for Parse. Fill header fields using data in given buffer,
	// but does not touch packet own buffer, leaving packet in invalid state.
	bool ParseBuffer(const uint8_t* buffer, size_t size);

	ExtensionInfo& FindOrCreateExtensionInfo(int id);
	// Returns pointer to extension info for a given id. Returns nullptr if not
	// found.
	const ExtensionInfo* FindExtensionInfo(int id) const;

	// Allocates and returns place to store rtp header extension.
	// Returns empty arrayview on failure.
	mediaserver::ArrayView<uint8_t> AllocateRawExtension(int id, size_t length);

	// Promotes existing one-byte header extensions to two-byte header extensions
	// by rewriting the data and updates the corresponding extension offsets.
	void PromoteToTwoByteHeaderExtension();

	uint16_t SetExtensionLengthMaybeAddZeroPadding(size_t extensions_offset);

	uint8_t* WriteAt(size_t offset) {
		if ( buffer_.size() > offset ) {
			return buffer_.data() + offset;
		} else {
			return NULL;
		}
	}
	void WriteAt(size_t offset, uint8_t byte) {
		if ( buffer_.size() > offset ) {
			buffer_.data()[offset] = byte;
		}
	}
	const uint8_t* ReadAt(size_t offset) const {
		if ( buffer_.size() > offset ) {
			return buffer_.data() + offset;
		} else {
			return NULL;
		}
	}

	// Extension
	std::vector<ExtensionInfo> extension_entries_;
	ExtensionManager extensions_;
	size_t extensions_size_ = 0;  // Unaligned.
	CopyOnWriteBuffer buffer_;
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
