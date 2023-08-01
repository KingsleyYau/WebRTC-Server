/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTC_BASE_SYNCHRONIZATION_RW_LOCK_POSIX_H_
#define RTC_BASE_SYNCHRONIZATION_RW_LOCK_POSIX_H_

#include <pthread.h>

#include <rtp/base/rw_lock_wrapper.h>

namespace qpidnetwork {

class RWLockPosix : public RWLockWrapper {
 public:
  static RWLockPosix* Create();
  ~RWLockPosix() override;

  void AcquireLockExclusive() override;
  void ReleaseLockExclusive() override;

  void AcquireLockShared() override;
  void ReleaseLockShared() override;

 private:
  RWLockPosix();
  bool Init();

  pthread_rwlock_t lock_;
};

}  // namespace qpidnetwork

#endif  // RTC_BASE_SYNCHRONIZATION_RW_LOCK_POSIX_H_
