/*
 * CommonFunc.h
 *
 *  Created on: 2015-3-12
 *      Author: Samson.Fan
 * Description: 存放公共函数
 */

#ifndef COMMONFUNCDEFINE_H_
#define COMMONFUNCDEFINE_H_

#include <stdint.h>
#include <string>
#include <sstream>
using namespace std;

#define KB (1024)
#define MB (1024 * KB)
#define GB (1024 * MB)

#ifndef _WIN32
// 获取数组元素个数
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

// 判断文件是否存在
bool IsFileExist(const string& path);
// 判断目录是否存在
bool IsDirExist(const string& path);
// 新建目录
bool MakeDir(const string& path);
// 删除目录（包括目录中所有文件及文件夹）
bool RemoveDir(const string& path);
// 删除文件
bool RemoveFile(const string& path);
// 修改文件名
bool RenameFile(const string& srcPath, const string& desPath);
// 复制文件
bool CopyFile(const string& srcPath, const string& desPath);
// 清空目录（删除目录里所有文件及文件夹）
bool CleanDir(const string& path);

// 初始化random
bool InitRandom();
// 获取random数
int GetRandomValue();

#ifdef WIN32
	// include 头文件
	#include <windows.h>
	#include <stdio.h>
	#include <time.h>
	
	// define
	#define snprintf sprintf_s
	#define usleep(x) Sleep((x/1000))

	// function
	inline int gettimeofday(struct timeval *tp, void *tzp)
	{
		time_t clock;
		struct tm tm;
		SYSTEMTIME wtm;

		GetLocalTime(&wtm);
		tm.tm_year = wtm.wYear - 1900;
		tm.tm_mon = wtm.wMonth - 1;
		tm.tm_mday = wtm.wDay;
		tm.tm_hour = wtm.wHour;
		tm.tm_min = wtm.wMinute;
		tm.tm_sec  = wtm.wSecond;
		tm. tm_isdst = -1;
		clock = mktime(&tm);
		tp->tv_sec = clock;
		tp->tv_usec = wtm.wMilliseconds * 1000;

		return (0);
	}
#else
	// include 头文件
	#include <stdio.h>
	#include <stdlib.h>
    #include <stdint.h>
	#include <sys/time.h>
	#include <unistd.h>
	#include <errno.h>

	// define
//	#define Sleep(ms) usleep(ms * 1000)
	#define Sleep(ms) millisleep(ms)

#endif

inline void millisleep(uint32_t milliseconds) {
	struct timespec ts = {milliseconds / 1000, (milliseconds % 1000) * 1000000};
	while ((-1 == nanosleep(&ts, &ts)) && (EINTR == errno));
}

// 获取当前时间（Unix Timestamp ms）
inline long long getCurrentTime()
{
	long long result = 0;
	struct timeval tv;
	gettimeofday(&tv,NULL);
	result = (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return result;
}

inline long long DiffTime(long long start, long long end)
{
    return end - start;
//	return (end > start ? end - start : (unsigned long)-1 - end + start);
}

inline string ReadableSize(long long size) {
	std::stringstream ss;
	string unit = "B";
    double floatSize = (1.0 * size);
    if (floatSize > GB) {
        floatSize = floatSize / GB;
        unit = "GB";
    } else if (floatSize > MB) {
        floatSize = floatSize / MB;
        unit = "MB";
    } else if (floatSize > KB) {
        floatSize = floatSize / KB;
        unit = "KB";
    }
    ss.setf(ios::fixed);
    ss.precision(2);
    ss << floatSize << unit;
	ss.unsetf(ios_base::dec);
    return ss.str();
}

inline string ReadableBps(long long size) {
	std::stringstream ss;
	string unit = "bps";
    double floatSize = (1.0 * size);
    if (floatSize > GB) {
        floatSize = floatSize / GB;
        unit = "Gbps";
    } else if (floatSize > MB) {
        floatSize = floatSize / MB;
        unit = "Mbps";
    } else if (floatSize > KB) {
        floatSize = floatSize / KB;
        unit = "Kbps";
    }
    ss.setf(ios::fixed);
    ss.precision(2);
    ss << floatSize << unit;
	ss.unsetf(ios_base::dec);
    return ss.str();
}

#endif /* COMMONFUNCDEFINE_H_ */
