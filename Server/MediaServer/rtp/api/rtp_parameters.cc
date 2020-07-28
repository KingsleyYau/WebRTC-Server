/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include <rtp/api/rtp_parameters.h>

#include <algorithm>
#include <string>

#include <rtp/api/array_view.h>
#include <rtp/base/strings/string_builder.h>

namespace mediaserver {

const double kDefaultBitratePriority = 1.0;

RtcpFeedback::RtcpFeedback() = default;
RtcpFeedback::RtcpFeedback(RtcpFeedbackType type) :
		type(type) {
}
RtcpFeedback::RtcpFeedback(RtcpFeedbackType type,
		RtcpFeedbackMessageType message_type) :
		type(type), message_type(message_type) {
}
RtcpFeedback::RtcpFeedback(const RtcpFeedback& rhs) = default;
RtcpFeedback::~RtcpFeedback() = default;

RtpCodecCapability::RtpCodecCapability() = default;
RtpCodecCapability::~RtpCodecCapability() = default;

RtpHeaderExtensionCapability::RtpHeaderExtensionCapability() = default;
RtpHeaderExtensionCapability::RtpHeaderExtensionCapability(
		const std::string& uri) :
		uri(uri) {
}
RtpHeaderExtensionCapability::RtpHeaderExtensionCapability(
		const std::string& uri, int preferred_id) :
		uri(uri), preferred_id(preferred_id) {
}
RtpHeaderExtensionCapability::~RtpHeaderExtensionCapability() = default;

RtpExtension::RtpExtension() = default;
RtpExtension::RtpExtension(const std::string& uri, int id) :
		uri(uri), id(id) {
}
RtpExtension::RtpExtension(const std::string& uri, int id, bool encrypt) :
		uri(uri), id(id), encrypt(encrypt) {
}
RtpExtension::~RtpExtension() = default;

RtpFecParameters::RtpFecParameters() = default;
RtpFecParameters::RtpFecParameters(FecMechanism mechanism) :
		mechanism(mechanism) {
}
RtpFecParameters::RtpFecParameters(FecMechanism mechanism, uint32_t ssrc) :
		ssrc(ssrc), mechanism(mechanism) {
}
RtpFecParameters::RtpFecParameters(const RtpFecParameters& rhs) = default;
RtpFecParameters::~RtpFecParameters() = default;

RtpRtxParameters::RtpRtxParameters() = default;
RtpRtxParameters::RtpRtxParameters(uint32_t ssrc) :
		ssrc(ssrc) {
}
RtpRtxParameters::RtpRtxParameters(const RtpRtxParameters& rhs) = default;
RtpRtxParameters::~RtpRtxParameters() = default;

RtpEncodingParameters::RtpEncodingParameters() = default;
RtpEncodingParameters::RtpEncodingParameters(const RtpEncodingParameters& rhs) = default;
RtpEncodingParameters::~RtpEncodingParameters() = default;

RtpCodecParameters::RtpCodecParameters() = default;
RtpCodecParameters::RtpCodecParameters(const RtpCodecParameters& rhs) = default;
RtpCodecParameters::~RtpCodecParameters() = default;

RtpCapabilities::RtpCapabilities() = default;
RtpCapabilities::~RtpCapabilities() = default;

RtcpParameters::RtcpParameters() = default;
RtcpParameters::RtcpParameters(const RtcpParameters& rhs) = default;
RtcpParameters::~RtcpParameters() = default;

RtpParameters::RtpParameters() = default;
RtpParameters::RtpParameters(const RtpParameters& rhs) = default;
RtpParameters::~RtpParameters() = default;

std::string RtpExtension::ToString() const {
	char buf[256];
	SimpleStringBuilder sb(buf);
	sb << "{uri: " << uri;
	sb << ", id: " << id;
	if (encrypt) {
		sb << ", encrypt";
	}
	sb << '}';
	return sb.str();
}

const char RtpExtension::kAudioLevelUri[] =
		"urn:ietf:params:rtp-hdrext:ssrc-audio-level";

const char RtpExtension::kTimestampOffsetUri[] =
		"urn:ietf:params:rtp-hdrext:toffset";

const char RtpExtension::kAbsSendTimeUri[] =
		"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time";

const char RtpExtension::kAbsoluteCaptureTimeUri[] =
		"http://www.webrtc.org/experiments/rtp-hdrext/abs-capture-time";

const char RtpExtension::kVideoRotationUri[] = "urn:3gpp:video-orientation";

const char RtpExtension::kTransportSequenceNumberUri[] =
		"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01";
const char RtpExtension::kTransportSequenceNumberV2Uri[] =
		"http://www.webrtc.org/experiments/rtp-hdrext/transport-wide-cc-02";

// This extension allows applications to adaptively limit the playout delay
// on frames as per the current needs. For example, a gaming application
// has very different needs on end-to-end delay compared to a video-conference
// application.
const char RtpExtension::kPlayoutDelayUri[] =
		"http://www.webrtc.org/experiments/rtp-hdrext/playout-delay";

const char RtpExtension::kVideoContentTypeUri[] =
		"http://www.webrtc.org/experiments/rtp-hdrext/video-content-type";

const char RtpExtension::kVideoTimingUri[] =
		"http://www.webrtc.org/experiments/rtp-hdrext/video-timing";

const char RtpExtension::kMidUri[] = "urn:ietf:params:rtp-hdrext:sdes:mid";

const char RtpExtension::kFrameMarkingUri[] =
		"http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07";

const char RtpExtension::kGenericFrameDescriptorUri00[] =
		"http://www.webrtc.org/experiments/rtp-hdrext/generic-frame-descriptor-00";
const char RtpExtension::kGenericFrameDescriptorUri01[] =
		"http://www.webrtc.org/experiments/rtp-hdrext/generic-frame-descriptor-01";
const char RtpExtension::kGenericFrameDescriptorUri[] =
		"http://www.webrtc.org/experiments/rtp-hdrext/generic-frame-descriptor-00";

const char RtpExtension::kEncryptHeaderExtensionsUri[] =
		"urn:ietf:params:rtp-hdrext:encrypt";

const char RtpExtension::kColorSpaceUri[] =
		"http://www.webrtc.org/experiments/rtp-hdrext/color-space";

const char RtpExtension::kRidUri[] =
		"urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id";

const char RtpExtension::kRepairedRidUri[] =
		"urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id";

constexpr int RtpExtension::kMinId;
constexpr int RtpExtension::kMaxId;
constexpr int RtpExtension::kMaxValueSize;
constexpr int RtpExtension::kOneByteHeaderExtensionMaxId;
constexpr int RtpExtension::kOneByteHeaderExtensionMaxValueSize;

bool RtpExtension::IsSupportedForAudio(const std::string& uri) {
	return uri == RtpExtension::kAudioLevelUri
			|| uri == RtpExtension::kAbsSendTimeUri
			||
			// TODO(bugs.webrtc.org/10739): Uncomment once the audio impl is ready.
			// uri == RtpExtension::kAbsoluteCaptureTimeUri ||
			uri == RtpExtension::kTransportSequenceNumberUri
			|| uri == RtpExtension::kTransportSequenceNumberV2Uri
			|| uri == RtpExtension::kMidUri || uri == RtpExtension::kRidUri
			|| uri == RtpExtension::kRepairedRidUri;
}

bool RtpExtension::IsSupportedForVideo(const std::string& uri) {
	return uri == RtpExtension::kTimestampOffsetUri
			|| uri == RtpExtension::kAbsSendTimeUri
			||
			// TODO(bugs.webrtc.org/10739): Uncomment once the video impl is ready.
			// uri == RtpExtension::kAbsoluteCaptureTimeUri ||
			uri == RtpExtension::kVideoRotationUri
			|| uri == RtpExtension::kTransportSequenceNumberUri
			|| uri == RtpExtension::kTransportSequenceNumberV2Uri
			|| uri == RtpExtension::kPlayoutDelayUri
			|| uri == RtpExtension::kVideoContentTypeUri
			|| uri == RtpExtension::kVideoTimingUri
			|| uri == RtpExtension::kMidUri
			|| uri == RtpExtension::kFrameMarkingUri
			|| uri == RtpExtension::kGenericFrameDescriptorUri00
			|| uri == RtpExtension::kGenericFrameDescriptorUri01
			|| uri == RtpExtension::kColorSpaceUri
			|| uri == RtpExtension::kRidUri
			|| uri == RtpExtension::kRepairedRidUri;
}

bool RtpExtension::IsEncryptionSupported(const std::string& uri) {
	return uri == RtpExtension::kAudioLevelUri
			|| uri == RtpExtension::kTimestampOffsetUri
			||
#if !defined(ENABLE_EXTERNAL_AUTH)
			// TODO(jbauch): Figure out a way to always allow "kAbsSendTimeUri"
			// here and filter out later if external auth is really used in
			// srtpfilter. External auth is used by Chromium and replaces the
			// extension header value of "kAbsSendTimeUri", so it must not be
			// encrypted (which can't be done by Chromium).
			uri == RtpExtension::kAbsSendTimeUri ||
#endif
			uri == RtpExtension::kAbsoluteCaptureTimeUri
			|| uri == RtpExtension::kVideoRotationUri
			|| uri == RtpExtension::kTransportSequenceNumberUri
			|| uri == RtpExtension::kTransportSequenceNumberV2Uri
			|| uri == RtpExtension::kPlayoutDelayUri
			|| uri == RtpExtension::kVideoContentTypeUri
			|| uri == RtpExtension::kMidUri || uri == RtpExtension::kRidUri
			|| uri == RtpExtension::kRepairedRidUri;
}

const RtpExtension* RtpExtension::FindHeaderExtensionByUri(
		const std::vector<RtpExtension>& extensions, const std::string& uri) {
	for (const auto& extension : extensions) {
		if (extension.uri == uri) {
			return &extension;
		}
	}
	return nullptr;
}

std::vector<RtpExtension> RtpExtension::FilterDuplicateNonEncrypted(
		const std::vector<RtpExtension>& extensions) {
	std::vector<RtpExtension> filtered;
	for (auto extension = extensions.begin(); extension != extensions.end();
			++extension) {
		if (extension->encrypt) {
			filtered.push_back(*extension);
			continue;
		}

		// Only add non-encrypted extension if no encrypted with the same URI
		// is also present...
		if (std::any_of(extension + 1, extensions.end(),
				[&](const RtpExtension& check) {
					return extension->uri == check.uri;
				})) {
			continue;
		}

		// ...and has not been added before.
		if (!FindHeaderExtensionByUri(filtered, extension->uri)) {
			filtered.push_back(*extension);
		}
	}
	return filtered;
}
}  // namespace mediaserver
