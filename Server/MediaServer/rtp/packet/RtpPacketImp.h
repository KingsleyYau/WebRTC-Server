/*
 * RtpPacketImp.h
 *
 *  Created on: 2020/07/14
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_PACKET_RTPPACKETIMP_H_
#define RTP_PACKET_RTPPACKETIMP_H_

#include <unistd.h>
#include <stdint.h>

#include <vector>
using namespace std;

namespace mediaserver {
class RtpPacketImp {
public:
	RtpPacketImp();
	virtual ~RtpPacketImp();

	void Reset();
	bool Parse(const uint8_t* buffer, size_t size);

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

	// Extension
	uint16_t transport_sequence_number_;

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
	std::vector<ExtensionInfo> extension_entries_;
	size_t extensions_size_ = 0;  // Unaligned.
};

} /* namespace mediaserver */

#endif /* RTP_PACKET_RTPPACKETIMP_H_ */
