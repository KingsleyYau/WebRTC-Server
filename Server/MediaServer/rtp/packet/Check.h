/*
 * Check.h
 *
 *  Created on: 2020/07/15
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_PACKET_CHECK_H_
#define RTP_PACKET_CHECK_H_

#include <common/LogManager.h>

#define RTC_CHECK(condition)											 \
  do {                                                                   \
    if (!(condition)) {													 \
		LogAync(LOG_DEBUG, "CHECK failed: " #condition);				 \
    }                                                                    \
  } while (0)

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

#endif /* RTP_PACKET_CHECK_H_ */
