/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */
#include <api/media_types.h>

#include <rtp/base/checks.h>

namespace cricket {

const char kMediaTypeVideo[] = "video";
const char kMediaTypeAudio[] = "audio";
const char kMediaTypeData[] = "data";

std::string MediaTypeToString(MediaType type) {
	switch (type) {
	case MEDIA_TYPE_AUDIO:
		return kMediaTypeAudio;
	case MEDIA_TYPE_VIDEO:
		return kMediaTypeVideo;
	case MEDIA_TYPE_DATA:
		return kMediaTypeData;
	}
	// Not reachable; avoids compile warning.
	return "";
}

}  // namespace cricket
