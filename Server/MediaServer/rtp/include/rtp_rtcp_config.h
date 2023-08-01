/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */


#ifndef RTP_INCLUDE_RTP_RTCP_CONFIG_H_
#define RTP_INCLUDE_RTP_RTCP_CONFIG_H_

// Configuration file for RTP utilities (RTPSender, RTPReceiver ...)
namespace qpidnetwork {
enum { kDefaultMaxReorderingThreshold = 50 };  // In sequence numbers.
enum { kRtcpMaxNackFields = 253 };

enum { RTCP_SEND_BEFORE_KEY_FRAME_MS = 100 };
enum { RTCP_MAX_REPORT_BLOCKS = 31 };  // RFC 3550 page 37
}  // namespace qpidnetwork

#endif  // RTP_INCLUDE_RTP_RTCP_CONFIG_H_
