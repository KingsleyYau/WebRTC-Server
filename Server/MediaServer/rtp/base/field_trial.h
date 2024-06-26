/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_BASE_FIELD_TRIAL_H_
#define RTP_BASE_FIELD_TRIAL_H_

#include <string>

// Field trials allow webrtc clients (such as Chrome) to turn on feature code
// in binaries out in the field and gather information with that.
//
// By default WebRTC provides an implementaion of field trials that can be
// found in system_wrappers/source/field_trial.cc. If clients want to provide
// a custom version, they will have to:
//
// 1. Compile WebRTC defining the preprocessor macro
//    WEBRTC_EXCLUDE_FIELD_TRIAL_DEFAULT (if GN is used this can be achieved
//    by setting the GN arg rtc_exclude_field_trial_default to true).
// 2. Provide an implementation of:
//    std::string webrtc::field_trial::FindFullName(const std::string& trial).
//
// They are designed to wire up directly to chrome field trials and to speed up
// developers by reducing the need to wire APIs to control whether a feature is
// on/off. E.g. to experiment with a new method that could lead to a different
// trade-off between CPU/bandwidth:
//
// 1 - Develop the feature with default behaviour off:
//
//   if (FieldTrial::FindFullName("WebRTCExperimentMethod2") == "Enabled")
//     method2();
//   else
//     method1();
//
// 2 - Once the changes are rolled to chrome, the new code path can be
//     controlled as normal chrome field trials.
//
// 3 - Evaluate the new feature and clean the code paths.
//
// Notes:
//   - NOT every feature is a candidate to be controlled by this mechanism as
//     it may require negotation between involved parties (e.g. SDP).
//
// TODO(andresp): since chrome --force-fieldtrials does not marks the trial
//     as active it does not gets propaged to renderer process. For now one
//     needs to push a config with start_active:true or run a local finch
//     server.
//
// TODO(andresp): find out how to get bots to run tests with trials enabled.

namespace qpidnetwork {
namespace field_trial {

// Returns the group name chosen for the named trial, or the empty string
// if the trial does not exists.
//
// Note: To keep things tidy append all the trial names with WebRTC.
std::string FindFullName(const std::string& name);

// Convenience method, returns true iff FindFullName(name) return a string that
// starts with "Enabled".
// TODO(tommi): Make sure all implementations support this.
inline bool IsEnabled(const char* name) {
	return FindFullName(name).find("Enabled") == 0;
}

// Convenience method, returns true iff FindFullName(name) return a string that
// starts with "Disabled".
inline bool IsDisabled(const char* name) {
	return FindFullName(name).find("Disabled") == 0;
}

// Optionally initialize field trial from a string.
// This method can be called at most once before any other call into webrtc.
// E.g. before the peer connection factory is constructed.
// Note: trials_string must never be destroyed.
void InitFieldTrialsFromString(const char* trials_string);

const char* GetFieldTrialString();

}  // namespace field_trial
}  // namespace qpidnetwork

#endif  // RTP_BASE_FIELD_TRIAL_H_
