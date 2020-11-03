/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include <rtp/packet/rtp_packet_to_send.h>

#include <cstdint>

namespace mediaserver {

RtpPacketToSend::RtpPacketToSend(const ExtensionManager* extensions) :
		RtpPacket(extensions) {
}
RtpPacketToSend::RtpPacketToSend(const ExtensionManager* extensions,
                                 size_t capacity)
    : RtpPacket(extensions, capacity) {}
RtpPacketToSend::RtpPacketToSend(const RtpPacketToSend& packet) = default;
RtpPacketToSend::RtpPacketToSend(RtpPacketToSend&& packet) = default;

RtpPacketToSend& RtpPacketToSend::operator=(const RtpPacketToSend& packet) = default;
RtpPacketToSend& RtpPacketToSend::operator=(RtpPacketToSend&& packet) = default;

RtpPacketToSend::~RtpPacketToSend() = default;

}  // namespace mediaserver
