/*
 * RtpPacket.cpp
 *
 *  Created on: 2020/07/2514
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include <rtp/packet/RtpPacket.h>

#include <common/LogManager.h>

namespace mediaserver {
namespace {
constexpr size_t kFixedHeaderSize = 12;
constexpr uint8_t kRtpVersion = 2;
constexpr uint16_t kOneByteExtensionProfileId = 0xBEDE;
constexpr uint16_t kTwoByteExtensionProfileId = 0x1000;
constexpr size_t kOneByteExtensionHeaderLength = 1;
constexpr size_t kTwoByteExtensionHeaderLength = 2;
constexpr size_t kDefaultPacketSize = 1500;
}

RtpPacket::RtpPacket() {
	// TODO Auto-generated constructor stub
	Reset();
}

RtpPacket::RtpPacket(const ExtensionManager* extensions) :
		extensions_(extensions ? *extensions : ExtensionManager()) {
	Reset();
}

RtpPacket::~RtpPacket() {
	// TODO Auto-generated destructor stub
}

void RtpPacket::Reset() {
	has_padding_ = false;
	has_extension_ = false;
	number_of_crcs_ = 0;
	marker_ = false;
	payload_type_ = 0;
	sequence_number_ = 0;
	timestamp_ = 0;
	ssrc_ = 0;
	payload_offset_ = kFixedHeaderSize;
	payload_size_ = 0;
	padding_size_ = 0;

	extensions_size_ = 0;
	extension_entries_.clear();
}

bool RtpPacket::Parse(const uint8_t* buffer, size_t size) {
	if (size < kFixedHeaderSize) {
		LogAync(LOG_WARNING, "RtpPacket::Parse( "
				"this : %p, "
				"[RTP packet header size error], "
				"size : %u "
				")", this, size);
		return false;
	}
	const uint8_t version = buffer[0] >> 6;
	if (version != kRtpVersion) {
		LogAync(LOG_WARNING, "RtpPacket::Parse( "
				"this : %p, "
				"[RTP version error], "
				"version : %u "
				")", this, version);
		return false;
	}
	has_padding_ = (buffer[0] & 0x20) != 0;
	has_extension_ = (buffer[0] & 0x10) != 0;
	number_of_crcs_ = buffer[0] & 0x0f;
	marker_ = (buffer[1] & 0x80) != 0;
	payload_type_ = buffer[1] & 0x7f;

	sequence_number_ = ByteReader<uint16_t>::ReadBigEndian(&buffer[2]);
	timestamp_ = ByteReader<uint32_t>::ReadBigEndian(&buffer[4]);
	ssrc_ = ByteReader<uint32_t>::ReadBigEndian(&buffer[8]);
	if (size < kFixedHeaderSize + number_of_crcs_ * 4) {
		LogAync(LOG_WARNING, "RtpPacket::Parse( "
				"this : %p, "
				"[RTP crcs size error], "
				"number_of_crcs : %u, "
				"minSize : %u, "
				"size : %u "
				")", this, number_of_crcs_,
				kFixedHeaderSize + number_of_crcs_ * 4, size);
		return false;
	}
	payload_offset_ = kFixedHeaderSize + number_of_crcs_ * 4;

	if (has_padding_) {
		padding_size_ = buffer[size - 1];
		if (padding_size_ == 0) {
			LogAync(LOG_WARNING, "RtpPacket::Parse( "
					"this : %p, "
					"[Padding was set, but padding size is zero], "
					"ssrc : 0x%08x(%u), "
					"padding_size : %u "
					")", this, ssrc_, ssrc_, padding_size_);
			return false;
		}
	} else {
		padding_size_ = 0;
	}

	extensions_size_ = 0;
	extension_entries_.clear();
	if (has_extension_) {
		/* RTP header extension, RFC 3550.
		 0                   1                   2                   3
		 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 |      defined by profile       |           length              |
		 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 |                        header extension                       |
		 |                             ....                              |
		 */
		size_t extension_offset = payload_offset_ + 4;
		if (extension_offset > size) {
			LogAync(LOG_WARNING, "RtpPacket::Parse( "
					"this : %p, "
					"[Unsupported rtp extension offset], "
					"size : %u, "
					"extension_offset : %u "
					")", this, size, extension_offset);
			return false;
		}
		uint16_t profile = ByteReader<uint16_t>::ReadBigEndian(
				&buffer[payload_offset_]);
		size_t extensions_capacity = ByteReader<uint16_t>::ReadBigEndian(
				&buffer[payload_offset_ + 2]);
		extensions_capacity *= 4;
		if (extension_offset + extensions_capacity > size) {
			return false;
		}
		if (profile != kOneByteExtensionProfileId
				&& profile != kTwoByteExtensionProfileId) {
			LogAync(LOG_WARNING, "RtpPacket::Parse( "
					"this : %p, "
					"[Unsupported rtp extension], "
					"profile : %u "
					")", this, profile);
		} else {
			size_t extension_header_length =
					profile == kOneByteExtensionProfileId ?
							kOneByteExtensionHeaderLength :
							kTwoByteExtensionHeaderLength;
			constexpr uint8_t kPaddingByte = 0;
			constexpr uint8_t kPaddingId = 0;
			constexpr uint8_t kOneByteHeaderExtensionReservedId = 15;
			while (extensions_size_ + extension_header_length
					< extensions_capacity) {
				if (buffer[extension_offset + extensions_size_]
						== kPaddingByte) {
					extensions_size_++;
					continue;
				}
				int id;
				uint8_t length;
				if (profile == kOneByteExtensionProfileId) {
					id = buffer[extension_offset + extensions_size_] >> 4;
					uint8_t ext = buffer[extension_offset + extensions_size_];
					length = 1 + (ext & 0xf);
					if (id == kOneByteHeaderExtensionReservedId
							|| (id == kPaddingId && length != 1)) {
						break;
					}
				} else {
					id = buffer[extension_offset + extensions_size_];
					length = buffer[extension_offset + extensions_size_ + 1];
				}

				if (extensions_size_ + extension_header_length + length
						> extensions_capacity) {
					LogAync(LOG_WARNING, "RtpPacket::Parse( "
							"this : %p, "
							"[Extension, Oversized rtp header extension], "
							"extensions_capacity : %u, "
							")", this, extensions_capacity);
					break;
				}

				ExtensionInfo& extension_info = FindOrCreateExtensionInfo(id);
				if (extension_info.length != 0) {
					LogAync(LOG_WARNING,
							"RtpPacket::Parse( "
									"this : %p, "
									"[Extension, Duplicate rtp header extension id, Overwriting], "
									"id : %d "
									")", this, id);
				}

				LogAync(LOG_DEBUG, "RtpPacket::Parse( "
						"this : %p, "
						"[Extension], "
						"ssrc : 0x%08x(%u), "
						"seq : %u, "
						"ts : %u, "
						"id : %u, "
						"length : %u "
						")", this, ssrc_, ssrc_, sequence_number_, timestamp_,
						id, length);

				size_t offset = extension_offset + extensions_size_
						+ extension_header_length;
				if (offset > (uint16_t) 0xFFFF) {
					LogAync(LOG_WARNING, "RtpPacket::Parse( "
							"this : %p, "
							"[Oversized rtp header extension], "
							"offset : %u "
							")", this, offset);
					break;
				}
				extension_info.offset = static_cast<uint16_t>(offset);
				extension_info.length = length;

//				RtpExtensionMap::const_iterator itr = rtp_extension_map_.find(
//						extension_info.id);
//				if (itr != rtp_extension_map_.end()) {
//					if (itr->second.uri == TransportSequenceNumber::kUri) {
//						hasTransportSequenceNumber_ = true;
//						transport_sequence_number_ = 0;
//						TransportSequenceNumber::Parse(
//								buffer + extension_info.offset,
//								extension_info.length,
//								&transport_sequence_number_);
//						LogAync(LOG_DEBUG, "RtpPacket::Parse( "
//								"this : %p, "
//								"[Extension, TransportSequenceNumber], "
//								"ssrc : 0x%08x(%u), "
//								"seq : %u, "
//								"ts : %u, "
//								"transport_sequence_number : %u "
//								")", this, ssrc_, ssrc_, sequence_number_,
//								timestamp_, transport_sequence_number_);
//					} else if (itr->second.uri == AbsoluteSendTime::kUri) {
//						AbsoluteSendTime::Parse(buffer + extension_info.offset,
//								extension_info.length, &abs_send_time_);
//						LogAync(LOG_DEBUG, "RtpPacket::Parse( "
//								"this : %p, "
//								"[Extension, AbsoluteSendTime], "
//								"ssrc : 0x%08x(%u), "
//								"seq : %u, "
//								"ts : %u, "
//								"abs_send_time : %u "
//								")", this, ssrc_, ssrc_, sequence_number_,
//								timestamp_, abs_send_time_);
//					} else {
//						LogAync(LOG_DEBUG, "RtpPacket::Parse( "
//								"this : %p, "
//								"[Extension, Unsupport], "
//								"ssrc : 0x%08x(%u), "
//								"seq : %u, "
//								"ts : %u "
//								")", this, ssrc_, ssrc_, sequence_number_,
//								timestamp_);
//					}
//				}

				extensions_size_ += extension_header_length + length;
			}
		}
		payload_offset_ = extension_offset + extensions_capacity;
	}

	if (payload_offset_ + padding_size_ > size) {
		return false;
	}
	payload_size_ = size - payload_offset_ - padding_size_;
	parse_data_ = buffer;
	return true;
}

std::vector<uint32_t> RtpPacket::Csrcs() const {
	size_t num_csrc = data()[0] & 0x0F;
	RTC_DCHECK_GE(capacity(), kFixedHeaderSize + num_csrc * 4);
	std::vector<uint32_t> csrcs(num_csrc);
	for (size_t i = 0; i < num_csrc; ++i) {
		csrcs[i] = ByteReader<uint32_t>::ReadBigEndian(
				&data()[kFixedHeaderSize + i * 4]);
	}
	return csrcs;
}

RtpPacket::ExtensionInfo& RtpPacket::FindOrCreateExtensionInfo(int id) {
	for (ExtensionInfo& extension : extension_entries_) {
		if (extension.id == id) {
			return extension;
		}
	}
	extension_entries_.emplace_back(id);
	return extension_entries_.back();
}

const RtpPacket::ExtensionInfo* RtpPacket::FindExtensionInfo(int id) const {
	for (const ExtensionInfo& extension : extension_entries_) {
		if (extension.id == id) {
			return &extension;
		}
	}
	return nullptr;
}

mediaserver::ArrayView<const uint8_t> RtpPacket::FindExtension(
		ExtensionType type) const {
	uint8_t id = extensions_.GetId(type);
	if (id == ExtensionManager::kInvalidId) {
		// Extension not registered.
		return nullptr;
	}
	ExtensionInfo const* extension_info = FindExtensionInfo(id);
	if (extension_info == nullptr) {
		return nullptr;
	}
	return mediaserver::MakeArrayView(data() + extension_info->offset,
			extension_info->length);
}

} /* namespace mediaserver */
