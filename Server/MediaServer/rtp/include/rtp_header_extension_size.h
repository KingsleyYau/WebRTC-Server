#ifndef RTP_RTP_HEADER_EXTENSION_SIZE_H_
#define RTP_RTP_HEADER_EXTENSION_SIZE_H_

#include <rtp/api/array_view.h>
#include <rtp/include/rtp_header_extension_map.h>
#include <rtp/include/rtp_rtcp_defines.h>

namespace mediaserver {

struct RtpExtensionSize {
	RTPExtensionType type;
	int value_size;
};

// Calculates rtp header extension size in bytes assuming packet contain
// all |extensions| with provided |value_size|.
// Counts only extensions present among |registered_extensions|.
int RtpHeaderExtensionSize(mediaserver::ArrayView<const RtpExtensionSize> extensions,
		const RtpHeaderExtensionMap& registered_extensions);

}  // namespace mediaserver

#endif  // RTP_RTP_HEADER_EXTENSION_SIZE_H_
