/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_BASE_REF_COUNTED_OBJECT_H_
#define RTP_BASE_REF_COUNTED_OBJECT_H_

#include <type_traits>
#include <utility>

#include <rtp/base/constructor_magic.h>
#include <rtp/base/ref_count.h>
#include <rtp/base/ref_counter.h>

namespace qpidnetwork {

template<class T>
class RefCountedObject: public T {
public:
	RefCountedObject() {
	}

	template<class P0>
	explicit RefCountedObject(P0&& p0) :
			T(std::forward<P0>(p0)) {
	}

	template<class P0, class P1, class ... Args>
	RefCountedObject(P0&& p0, P1&& p1, Args&&... args) :
			T(std::forward<P0>(p0), std::forward<P1>(p1),
					std::forward<Args>(args)...) {
	}

	virtual void AddRef() const {
		ref_count_.IncRef();
	}

	virtual RefCountReleaseStatus Release() const {
		const auto status = ref_count_.DecRef();
		if (status == RefCountReleaseStatus::kDroppedLastRef) {
			delete this;
		}
		return status;
	}

	// Return whether the reference count is one. If the reference count is used
	// in the conventional way, a reference count of 1 implies that the current
	// thread owns the reference and no other thread shares it. This call
	// performs the test for a reference count of one, and performs the memory
	// barrier needed for the owning thread to act on the object, knowing that it
	// has exclusive access to the object.
	virtual bool HasOneRef() const {
		return ref_count_.HasOneRef();
	}

protected:
	virtual ~RefCountedObject() {
	}

	mutable qpidnetwork::mediaserver_impl::RefCounter ref_count_ { 0 };

	RTC_DISALLOW_COPY_AND_ASSIGN(RefCountedObject);
};

}  // namespace qpidnetwork

#endif  // RTP_BASE_REF_COUNTED_OBJECT_H_
