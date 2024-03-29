/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_BASE_REF_COUNT_H_
#define RTP_BASE_REF_COUNT_H_

namespace qpidnetwork {

// Refcounted objects should implement the following informal interface:
//
// void AddRef() const ;
// RefCountReleaseStatus Release() const;
//
// You may access members of a reference-counted object, including the AddRef()
// and Release() methods, only if you already own a reference to it, or if
// you're borrowing someone else's reference. (A newly created object is a
// special case: the reference count is zero on construction, and the code that
// creates the object should immediately call AddRef(), bringing the reference
// count from zero to one, e.g., by constructing an qpidnetwork::scoped_refptr).
//
// AddRef() creates a new reference to the object.
//
// Release() releases a reference to the object; the caller now has one less
// reference than before the call. Returns kDroppedLastRef if the number of
// references dropped to zero because of this (in which case the object destroys
// itself). Otherwise, returns kOtherRefsRemained, to signal that at the precise
// time the caller's reference was dropped, other references still remained (but
// if other threads own references, this may of course have changed by the time
// Release() returns).
//
// The caller of Release() must treat it in the same way as a delete operation:
// Regardless of the return value from Release(), the caller mustn't access the
// object. The object might still be alive, due to references held by other
// users of the object, but the object can go away at any time, e.g., as the
// result of another thread calling Release().
//
// Calling AddRef() and Release() manually is discouraged. It's recommended to
// use qpidnetwork::scoped_refptr to manage all pointers to reference counted objects.
// Note that qpidnetwork::scoped_refptr depends on compile-time duck-typing; formally
// implementing the below RefCountInterface is not required.

enum class RefCountReleaseStatus {
	kDroppedLastRef, kOtherRefsRemained
};

// Interfaces where refcounting is part of the public api should
// inherit this abstract interface. The implementation of these
// methods is usually provided by the RefCountedObject template class,
// applied as a leaf in the inheritance tree.
class RefCountInterface {
public:
	virtual void AddRef() const = 0;
	virtual RefCountReleaseStatus Release() const = 0;

	// Non-public destructor, because Release() has exclusive responsibility for
	// destroying the object.
protected:
	virtual ~RefCountInterface() {
	}
};

}  // namespace qpidnetwork

#endif  // RTP_BASE_REF_COUNT_H_
