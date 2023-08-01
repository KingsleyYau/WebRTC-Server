/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include <rtp/api/transport/field_trial_based_config.h>

#include <rtp/base/field_trial.h>

namespace qpidnetwork {
std::string FieldTrialBasedConfig::Lookup(absl::string_view key) const {
	return field_trial::FindFullName(std::string(key));
}
}  // namespace qpidnetwork
