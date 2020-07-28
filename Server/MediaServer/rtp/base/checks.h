/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_BASE_CHECK_H_
#define RTP_BASE_CHECK_H_

#include <common/LogManager.h>

#define RTC_DCHECK(condition)
#define RTC_DCHECK_EQ(a, b) RTC_DCHECK((a) == (b))
#define RTC_DCHECK_NE(a, b) RTC_DCHECK((a) != (b))
#define RTC_DCHECK_LE(a, b) RTC_DCHECK((a) <= (b))
#define RTC_DCHECK_LT(a, b) RTC_DCHECK((a) < (b))
#define RTC_DCHECK_GE(a, b) RTC_DCHECK((a) >= (b))
#define RTC_DCHECK_GT(a, b) RTC_DCHECK((a) > (b))

#define RTC_CHECK(condition)

#define RTC_CHECK_EQ(a, b) RTC_CHECK((a) == (b))
#define RTC_CHECK_NE(a, b) RTC_CHECK((a) != (b))
#define RTC_CHECK_LE(a, b) RTC_CHECK((a) <= (b))
#define RTC_CHECK_LT(a, b) RTC_CHECK((a) < (b))
#define RTC_CHECK_GE(a, b) RTC_CHECK((a) >= (b))
#define RTC_CHECK_GT(a, b) RTC_CHECK((a) > (b))

template <typename T>
inline T CheckedDivExact(T a, T b) {
  return a / b;
}

#endif /* RTP_BASE_CHECK_H_ */
