/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include <string.h>

#include <rtp/base/checks.h>
#include <rtp/base/zero_memory.h>

namespace qpidnetwork {

// Code and comment taken from "OPENSSL_cleanse" of BoringSSL.
void ExplicitZeroMemory(void* ptr, size_t len) {
	RTC_DCHECK(ptr || !len);
	memset(ptr, 0, len);
#if !defined(__pnacl__)
	/* As best as we can tell, this is sufficient to break any optimisations that
	 might try to eliminate "superfluous" memsets. If there's an easy way to
	 detect memset_s, it would be better to use that. */
	__asm__ __volatile__("" : : "r"(ptr) : "memory");
	// NOLINT
#endif
}

}  // namespace qpidnetwork
