#include <rtp/api/rtp_headers.h>

namespace mediaserver {

//RTPHeaderExtension::RTPHeaderExtension() :
//		hasTransmissionTimeOffset(false), transmissionTimeOffset(0), hasAbsoluteSendTime(
//				false), absoluteSendTime(0), hasTransportSequenceNumber(false), transportSequenceNumber(
//				0), hasAudioLevel(false), voiceActivity(false), audioLevel(0), hasVideoRotation(
//				false), videoRotation(kVideoRotation_0), hasVideoContentType(
//				false), videoContentType(VideoContentType::UNSPECIFIED), has_video_timing(
//				false), has_frame_marking(false), frame_marking( { false, false,
//				false, false, false, 0xFF, 0, 0 }) {
//}
RTPHeaderExtension::RTPHeaderExtension() :
		hasTransmissionTimeOffset(false), transmissionTimeOffset(0), hasAbsoluteSendTime(
				false), absoluteSendTime(0), hasTransportSequenceNumber(false), transportSequenceNumber(
				0), hasAudioLevel(false), voiceActivity(false), audioLevel(0) {
}

RTPHeaderExtension::RTPHeaderExtension(const RTPHeaderExtension& other) = default;

RTPHeaderExtension& RTPHeaderExtension::operator=(
		const RTPHeaderExtension& other) = default;

RTPHeader::RTPHeader() :
		markerBit(false), payloadType(0), sequenceNumber(0), timestamp(0), ssrc(
				0), numCSRCs(0), arrOfCSRCs(), paddingLength(0), headerLength(
				0), payload_type_frequency(0), extension() {
}

RTPHeader::RTPHeader(const RTPHeader& other) = default;

RTPHeader& RTPHeader::operator=(const RTPHeader& other) = default;

}  // namespace mediaserver
