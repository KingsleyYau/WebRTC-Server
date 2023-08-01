/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include <rtp/base/rw_lock_posix.h>

#include <stddef.h>

namespace qpidnetwork {

RWLockPosix::RWLockPosix() : lock_() {}

RWLockPosix::~RWLockPosix() {
  pthread_rwlock_destroy(&lock_);
}

RWLockPosix* RWLockPosix::Create() {
  RWLockPosix* ret_val = new RWLockPosix();
  if (!ret_val->Init()) {
    delete ret_val;
    return NULL;
  }
  return ret_val;
}

bool RWLockPosix::Init() {
  return pthread_rwlock_init(&lock_, 0) == 0;
}

void RWLockPosix::AcquireLockExclusive() {
  pthread_rwlock_wrlock(&lock_);
}

void RWLockPosix::ReleaseLockExclusive() {
  pthread_rwlock_unlock(&lock_);
}

void RWLockPosix::AcquireLockShared() {
  pthread_rwlock_rdlock(&lock_);
}

void RWLockPosix::ReleaseLockShared() {
  pthread_rwlock_unlock(&lock_);
}

}  // namespace qpidnetwork
