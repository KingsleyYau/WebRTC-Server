#ifndef RTP_INCLUDE_RTP_HEADER_EXTENSION_MAP_H_
#define RTP_INCLUDE_RTP_HEADER_EXTENSION_MAP_H_

#include <stdint.h>

#include <string>

#include <rtp/api/array_view.h>
#include <rtp/api/rtp_parameters.h>
#include <rtp/include/rtp_rtcp_defines.h>
#include <rtp/base/checks.h>

namespace mediaserver {

class RtpHeaderExtensionMap {
public:
	static constexpr RTPExtensionType kInvalidType = kRtpExtensionNone;
	static constexpr int kInvalidId = 0;

	RtpHeaderExtensionMap();
	explicit RtpHeaderExtensionMap(bool extmap_allow_mixed);
	explicit RtpHeaderExtensionMap(ArrayView<const RtpExtension> extensions);

	template<typename Extension>
	bool Register(int id) {
		return Register(id, Extension::kId, Extension::kUri);
	}
	bool RegisterByType(int id, RTPExtensionType type);
	bool RegisterByUri(int id, absl::string_view uri);

	bool IsRegistered(RTPExtensionType type) const {
		return GetId(type) != kInvalidId;
	}
	// Return kInvalidType if not found.
	RTPExtensionType GetType(int id) const;
	// Return kInvalidId if not found.
	uint8_t GetId(RTPExtensionType type) const {
		RTC_DCHECK_GT(type, kRtpExtensionNone);RTC_DCHECK_LT(type, kRtpExtensionNumberOfExtensions);
		return ids_[type];
	}

//  // TODO(danilchap): Remove use of the functions below.
//  RTC_DEPRECATED int32_t Register(RTPExtensionType type, int id) {
//    return RegisterByType(id, type) ? 0 : -1;
//  }
	int32_t Deregister(RTPExtensionType type);
	void Deregister(absl::string_view uri);

	// Corresponds to the SDP attribute extmap-allow-mixed, see RFC8285.
	// Set to true if it's allowed to mix one- and two-byte RTP header extensions
	// in the same stream.
	bool ExtmapAllowMixed() const {
		return extmap_allow_mixed_;
	}
	void SetExtmapAllowMixed(bool extmap_allow_mixed) {
		extmap_allow_mixed_ = extmap_allow_mixed;
	}

	void Clear();

private:
	bool Register(int id, RTPExtensionType type, const char* uri);

	uint8_t ids_[kRtpExtensionNumberOfExtensions];
	bool extmap_allow_mixed_;
};

}  // namespace mediaserver

#endif  // RTP_INCLUDE_RTP_HEADER_EXTENSION_MAP_H_
