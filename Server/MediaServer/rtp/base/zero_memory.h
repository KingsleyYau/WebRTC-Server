/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_BASE_ZERO_MEMORY_H_
#define RTP_BASE_ZERO_MEMORY_H_

#include <stddef.h>

#include <type_traits>

#include <rtp/api/array_view.h>

namespace mediaserver {

// Fill memory with zeros in a way that the compiler doesn't optimize it away
// even if the pointer is not used afterwards.
void ExplicitZeroMemory(void* ptr, size_t len);

template<typename T, typename std::enable_if<
		!std::is_const<T>::value && std::is_trivial<T>::value>::type* = nullptr>
void ExplicitZeroMemory(mediaserver::ArrayView<T> a) {
	ExplicitZeroMemory(a.data(), a.size());
}

}  // namespace mediaserver

#endif  // RTP_BASE_ZERO_MEMORY_H_
