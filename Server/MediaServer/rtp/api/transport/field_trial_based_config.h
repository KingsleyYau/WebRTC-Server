/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_API_TRANSPORT_FIELD_TRIAL_BASED_CONFIG_H_
#define RTP_API_TRANSPORT_FIELD_TRIAL_BASED_CONFIG_H_

#include <string>

#include <absl/strings/string_view.h>
#include <rtp/api/transport/webrtc_key_value_config.h>

namespace qpidnetwork {
// Implementation using the field trial API fo the key value lookup.
class FieldTrialBasedConfig: public WebRtcKeyValueConfig {
public:
	std::string Lookup(absl::string_view key) const override;
};
}  // namespace qpidnetwork

#endif  // RTP_API_TRANSPORT_FIELD_TRIAL_BASED_CONFIG_H_
