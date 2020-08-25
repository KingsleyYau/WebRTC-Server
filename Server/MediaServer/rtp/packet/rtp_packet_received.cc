/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include <rtp/packet/rtp_packet_received.h>

#include <stddef.h>

#include <cstdint>
#include <vector>

#include <rtp/include/rtp_header_extensions.h>
#include <rtp/base/numerics/safe_conversions.h>

namespace mediaserver {

RtpPacketReceived::RtpPacketReceived() = default;
RtpPacketReceived::RtpPacketReceived(const ExtensionManager* extensions) :
		RtpPacket(extensions) {
}
RtpPacketReceived::RtpPacketReceived(const RtpPacketReceived& packet) = default;
RtpPacketReceived::RtpPacketReceived(RtpPacketReceived&& packet) = default;

RtpPacketReceived& RtpPacketReceived::operator=(const RtpPacketReceived& packet) = default;
RtpPacketReceived& RtpPacketReceived::operator=(RtpPacketReceived&& packet) = default;

RtpPacketReceived::~RtpPacketReceived() {
}

void RtpPacketReceived::GetHeader(RTPHeader* header) const {
	header->markerBit = Marker();
	header->payloadType = PayloadType();
	header->sequenceNumber = SequenceNumber();
	header->timestamp = Timestamp();
	header->ssrc = Ssrc();
	std::vector < uint32_t > csrcs = Csrcs();
	header->numCSRCs = mediaserver::dchecked_cast < uint8_t > (csrcs.size());
	for (size_t i = 0; i < csrcs.size(); ++i) {
		header->arrOfCSRCs[i] = csrcs[i];
	}
	header->paddingLength = padding_size();
	header->headerLength = headers_size();
	header->payload_type_frequency = payload_type_frequency();
//	header->extension.hasTransmissionTimeOffset = GetExtension
//			< TransmissionOffset > (&header->extension.transmissionTimeOffset);
	header->extension.hasAbsoluteSendTime = GetExtension < AbsoluteSendTime
			> (&header->extension.absoluteSendTime);
//	header->extension.absolute_capture_time = GetExtension<
//			AbsoluteCaptureTimeExtension>();
//	header->extension.hasTransportSequenceNumber =
//			GetExtension < TransportSequenceNumberV2
//					> (&header->extension.transportSequenceNumber, &header->extension.feedback_request)
//					|| GetExtension < TransportSequenceNumber
//							> (&header->extension.transportSequenceNumber);
//	header->extension.hasAudioLevel = GetExtension < AudioLevel
//			> (&header->extension.voiceActivity, &header->extension.audioLevel);
//	header->extension.hasVideoRotation = GetExtension < VideoOrientation
//			> (&header->extension.videoRotation);
//	header->extension.hasVideoContentType = GetExtension
//			< VideoContentTypeExtension > (&header->extension.videoContentType);
//	header->extension.has_video_timing = GetExtension < VideoTimingExtension
//			> (&header->extension.video_timing);
//	header->extension.has_frame_marking = GetExtension < FrameMarkingExtension
//			> (&header->extension.frame_marking);
//	GetExtension < RtpStreamId > (&header->extension.stream_id);
//	GetExtension < RepairedRtpStreamId
//			> (&header->extension.repaired_stream_id);
//	GetExtension < RtpMid > (&header->extension.mid);
//	GetExtension < PlayoutDelayLimits > (&header->extension.playout_delay);
//	header->extension.color_space = GetExtension<ColorSpaceExtension>();
}

}  // namespace mediaserver
