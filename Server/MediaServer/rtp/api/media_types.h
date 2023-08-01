/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_API_MEDIA_TYPES_H_
#define RTP_API_MEDIA_TYPES_H_

#include <string>

#include <rtp/base/system/rtc_export.h>

// The cricket and webrtc have separate definitions for what a media type is.
// They're not compatible. Watch out for this.

namespace cricket {

enum MediaType {
	MEDIA_TYPE_AUDIO, MEDIA_TYPE_VIDEO, MEDIA_TYPE_DATA
};

extern const char kMediaTypeAudio[];
extern const char kMediaTypeVideo[];
extern const char kMediaTypeData[];

RTC_EXPORT std::string MediaTypeToString(MediaType type);

}  // namespace cricket

namespace qpidnetwork {

enum class MediaType {
	ANY, AUDIO, VIDEO, DATA
};

}  // namespace qpidnetwork

#endif  // RTP_API_MEDIA_TYPES_H_
