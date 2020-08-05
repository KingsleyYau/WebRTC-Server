/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_API_TRANSPORT_WEBRTC_KEY_VALUE_CONFIG_H_
#define RTP_API_TRANSPORT_WEBRTC_KEY_VALUE_CONFIG_H_

#include <string>

#include <absl/strings/string_view.h>

namespace mediaserver {

// An interface that provides a key-value mapping for configuring internal
// details of WebRTC. Note that there's no guarantess that the meaning of a
// particular key value mapping will be preserved over time and no announcements
// will be made if they are changed. It's up to the library user to ensure that
// the behavior does not break.
class WebRtcKeyValueConfig {
public:
	virtual ~WebRtcKeyValueConfig() = default;
	// The configured value for the given key. Defaults to an empty string.
	virtual std::string Lookup(absl::string_view key) const = 0;
};
}  // namespace mediaserver

#endif  // RTP_API_TRANSPORT_WEBRTC_KEY_VALUE_CONFIG_H_
