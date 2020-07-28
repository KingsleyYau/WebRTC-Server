/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include <rtp/base/rw_lock_wrapper.h>

#include <rtp/base/rw_lock_posix.h>

namespace mediaserver {

RWLockWrapper* RWLockWrapper::CreateRWLock() {
  return RWLockPosix::Create();
}

}  // namespace mediaserver
