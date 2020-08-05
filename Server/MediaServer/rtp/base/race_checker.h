/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_BASE_RACE_CHECKER_H_
#define RTP_BASE_RACE_CHECKER_H_

#include <rtp/base/checks.h>
#include <rtp/base/platform_thread_types.h>
#include <rtp/base/thread_annotations.h>

namespace mediaserver {

namespace internal {
class RaceCheckerScope;
}  // namespace internal

// Best-effort race-checking implementation. This primitive uses no
// synchronization at all to be as-fast-as-possible in the non-racy case.
class RTC_LOCKABLE RaceChecker {
public:
	friend class internal::RaceCheckerScope;
	RaceChecker();

private:
	bool Acquire() const RTC_EXCLUSIVE_LOCK_FUNCTION();
	void Release() const RTC_UNLOCK_FUNCTION();

	// Volatile to prevent code being optimized away in Acquire()/Release().
	mutable volatile int access_count_ = 0;
	mutable volatile PlatformThreadRef accessing_thread_;
};

namespace internal {
class RTC_SCOPED_LOCKABLE RaceCheckerScope {
public:
	explicit RaceCheckerScope(const RaceChecker* race_checker)
			RTC_EXCLUSIVE_LOCK_FUNCTION(race_checker);

	bool RaceDetected() const;
	~RaceCheckerScope() RTC_UNLOCK_FUNCTION();

private:
	const RaceChecker* const race_checker_;
	const bool race_check_ok_;
};

class RTC_SCOPED_LOCKABLE RaceCheckerScopeDoNothing {
public:
	explicit RaceCheckerScopeDoNothing(const RaceChecker* race_checker)
			RTC_EXCLUSIVE_LOCK_FUNCTION(race_checker) {
	}

	~RaceCheckerScopeDoNothing() RTC_UNLOCK_FUNCTION() {
	}
};

}  // namespace internal
}  // namespace rtc

#define RTC_CHECK_RUNS_SERIALIZED(x)               \
  mediaserver::internal::RaceCheckerScope race_checker(x); \
  RTC_CHECK(!race_checker.RaceDetected())

#if RTC_DCHECK_IS_ON
#define RTC_DCHECK_RUNS_SERIALIZED(x)              \
  mediaserver::internal::RaceCheckerScope race_checker(x); \
  RTC_DCHECK(!race_checker.RaceDetected())
#else
#define RTC_DCHECK_RUNS_SERIALIZED(x) \
  mediaserver::internal::RaceCheckerScopeDoNothing race_checker(x)
#endif

#endif  // RTC_BASE_RACE_CHECKER_H_
