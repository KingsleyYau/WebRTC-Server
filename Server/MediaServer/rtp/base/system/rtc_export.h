/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_BASE_SYSTEM_RTC_EXPORT_H_
#define RTP_BASE_SYSTEM_RTC_EXPORT_H_

// RTC_EXPORT is used to mark symbols as exported or imported when WebRTC is
// built or used as a shared library.
// When WebRTC is built as a static library the RTC_EXPORT macro expands to
// nothing.

#ifdef WEBRTC_ENABLE_SYMBOL_EXPORT

#ifdef WEBRTC_WIN

#ifdef WEBRTC_LIBRARY_IMPL
#define RTC_EXPORT __declspec(dllexport)
#else
#define RTC_EXPORT __declspec(dllimport)
#endif

#else  // WEBRTC_WIN

#if __has_attribute(visibility) && defined(WEBRTC_LIBRARY_IMPL)
#define RTC_EXPORT __attribute__((visibility("default")))
#endif

#endif  // WEBRTC_WIN

#endif  // WEBRTC_ENABLE_SYMBOL_EXPORT

#ifndef RTC_EXPORT
#define RTC_EXPORT
#endif

#endif  // RTP_BASE_SYSTEM_RTC_EXPORT_H_
