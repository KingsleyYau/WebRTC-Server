/*
 * LogManager.h
 *
 *  Created on: 2015-1-13
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef LOGMANAGER_H_
#define LOGMANAGER_H_

#include <common/KMutex.h>
#include <common/KThread.h>
#include <common/LogFile.hpp>

#include <string>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#define FLAG_2_LOG_IA(bFlag) bFlag?LOG_INFO:LOG_ALERT
#define FLAG_2_LOG_IW(bFlag) bFlag?LOG_INFO:LOG_WARN
#define FLAG_2_LOG_ID(bFlag) bFlag?LOG_INFO:LOG_DEBUG
#define FLAG_2_LOG_NA(bFlag) bFlag?LOG_NOTICE:LOG_ALERT
#define FLAG_2_LOG_NE(bFlag) bFlag?LOG_NOTICE:LOG_ERR
#define FLAG_2_LOG_NW(bFlag) bFlag?LOG_NOTICE:LOG_WARN
#define FLAG_2_LOG_ND(bFlag) bFlag?LOG_NOTICE:LOG_DEBUG

#define DiffGetTickCount(start, end)    ((start) <= (end) ? (end) - (start) : ((unsigned int)(-1)) - (start) + (end))

#define LogAyncUnSafe(level, fmt, ...) \
		LogManager::GetLogManager()->LogUnSafe(__FILE__, __LINE__, level, fmt, ## __VA_ARGS__)
#define LogAync(level, fmt, ...) \
		LogManager::GetLogManager()->Log(__FILE__, __LINE__, level, fmt, ## __VA_ARGS__)
#define LogAyncFunc(level, fmt, ...) \
		LogManager::GetLogManager()->Log(__FILE__, __LINE__, __FUNCTION__, level, fmt, ## __VA_ARGS__)

class LogRunnable;
class LogManager {
public:

	static LogManager *GetLogManager();
	static string LogLevelDesc(LOG_LEVEL nLevel);

	LogManager();
	virtual ~LogManager();

	bool Start(LOG_LEVEL nLevel = LOG_DEBUG, const string& dir = "log");
	bool Stop();
	bool IsRunning();
	bool Log(const char *file, int line, LOG_LEVEL nLevel, const char *format, ...);
	bool Log(const char *file, int line, const char *func, LOG_LEVEL nLevel, const char *format, ...);
	bool LogUnSafe(const char *file, int line, LOG_LEVEL nLevel, const char *format, ...);
	int MkDir(const char* pDir);
	void SetLogLevel(LOG_LEVEL nLevel = LOG_DEBUG);

	void LogSetFlushBuffer(unsigned int iLen);
	void LogFlushMem2File();
	void SetDebugMode(bool debugMode);
	void SetSTDMode(bool stdMode);

private:
	KThread mLogThread;
	LogRunnable *mpLogRunnable;
	bool mIsRunning;

	string mLogDir;

	KMutex mMutex;
	CFileCtrl *mpFileCtrl;
	LOG_LEVEL mLogLevel;

	CFileCtrl *mpFileCtrlDebug;
	bool mDebugMode;
	bool mSTDMode;
};

#endif /* LOGMANAGER_H_ */
