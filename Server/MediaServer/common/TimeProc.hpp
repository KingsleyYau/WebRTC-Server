/*
 * File         : TimeProc.hpp
 * Date         : 2012-10-19
 * Author       : FGX
 * Description  :
 */

#ifndef __TIMEPROC_DEF_H_
#define __TIMEPROC_DEF_H_

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <ctime>

using namespace std;


#define DAY_USECONDS	86400000
#define GetTickCountDifferences(start, end)    ((start) <= (end) ? (end) - (start) : DAY_USECONDS - (start) + (end))
inline unsigned int GetTickCount()
{
    struct timeval tv;
    if(gettimeofday(&tv, NULL) != 0) {
        return 0;
    }

    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

inline void GetLocalTimeString(char* szTime, int size, time_t tTime)
{
    struct tm *tmLocalTime = localtime(&tTime);
    snprintf(szTime, size, "%d-%d-%d %d:%d:%d"
        , 1900 + tmLocalTime->tm_year
        , tmLocalTime->tm_mon+1
        , tmLocalTime->tm_mday
        , tmLocalTime->tm_hour
        , tmLocalTime->tm_min
        , tmLocalTime->tm_sec);
}

inline string GetCurrentTimeString() {
    time_t timestamp = time(NULL);
    struct tm tTime;
    localtime_r(&timestamp, &tTime);

    struct timeval tv;
    gettimeofday(&tv, NULL);

    char bitBuffer[128] = {0};
    snprintf(bitBuffer, sizeof(bitBuffer) - 1,
    		"%d-%02d-%02d %02d:%02d:%02d",
    		tTime.tm_year+1900, tTime.tm_mon+1, tTime.tm_mday,
			tTime.tm_hour, tTime.tm_min, tTime.tm_sec
    		);

    return bitBuffer;
}

inline long long GetCurrentTimeamp() {
    time_t timestamp = time(NULL);
    return timestamp;
}

inline long long StringToTimestamp(const string& datetime_str, bool is_utc = false) {
	struct tm tTime;
    if (strptime(datetime_str.c_str(), "%Y%m%d%H%M%S", &tTime) == NULL) {
        return -1;
    }
    time_t timestamp = mktime(&tTime);
    return timestamp;
}

#endif
