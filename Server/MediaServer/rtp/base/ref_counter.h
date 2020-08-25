/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_BASE_REF_COUNTER_H_
#define RTP_BASE_REF_COUNTER_H_

#include <rtp/base/atomic_ops.h>
#include <rtp/base/ref_count.h>

namespace mediaserver {
namespace mediaserver_impl {

class RefCounter {
public:
	explicit RefCounter(int ref_count) :
			ref_count_(ref_count) {
	}
	RefCounter() = delete;

	void IncRef() {
		mediaserver::AtomicOps::Increment(&ref_count_);
	}

	// Returns kDroppedLastRef if this call dropped the last reference; the caller
	// should therefore free the resource protected by the reference counter.
	// Otherwise, returns kOtherRefsRemained (note that in case of multithreading,
	// some other caller may have dropped the last reference by the time this call
	// returns; all we know is that we didn't do it).
	mediaserver::RefCountReleaseStatus DecRef() {
		return (mediaserver::AtomicOps::Decrement(&ref_count_) == 0) ?
				mediaserver::RefCountReleaseStatus::kDroppedLastRef :
				mediaserver::RefCountReleaseStatus::kOtherRefsRemained;
	}

	// Return whether the reference count is one. If the reference count is used
	// in the conventional way, a reference count of 1 implies that the current
	// thread owns the reference and no other thread shares it. This call performs
	// the test for a reference count of one, and performs the memory barrier
	// needed for the owning thread to act on the resource protected by the
	// reference counter, knowing that it has exclusive access.
	bool HasOneRef() const {
		return mediaserver::AtomicOps::AcquireLoad(&ref_count_) == 1;
	}

private:
	volatile int ref_count_;
};

}  // namespace mediaserver_impl
}  // namespace mediaserver

#endif  // RTP_BASE_REF_COUNTER_H_
