/*
 * httpc.cpp
 *
 *  Created on: 2023-01-28
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include <string>
using namespace std;

// ThirdParty
#include <json/json.h>
// Common
#include <common/Math.h>
#include <common/CommonFunc.h>
#include <common/LogManager.h>
#include <common/Arithmetic.h>
#include <common/StringHandle.h>

#include <httpclient/HttpClient.h>

bool Parse(int argc, char *argv[]);
void SignalFunc(int signal);

int iLogLevel = LOG_INFO;

int main(int argc, char *argv[]) {
	printf("############## httpc ############## \n");
	printf("# Build date : %s %s \n", __DATE__, __TIME__);
	Parse(argc, argv);
	srand(time(0));

	/* Ignore */
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGPIPE, &sa, 0);

	/* Handle */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SignalFunc;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);

//	sigaction(SIGHUP, &sa, 0);
	sigaction(SIGINT, &sa, 0); // Ctrl-C
	sigaction(SIGQUIT, &sa, 0);
	sigaction(SIGILL, &sa, 0);
	sigaction(SIGABRT, &sa, 0);
	sigaction(SIGFPE, &sa, 0);
	sigaction(SIGBUS, &sa, 0);
	sigaction(SIGSEGV, &sa, 0);
	sigaction(SIGSYS, &sa, 0);
	sigaction(SIGTERM, &sa, 0);
	sigaction(SIGXCPU, &sa, 0);
	sigaction(SIGXFSZ, &sa, 0);
	// 回收子进程
	sigaction(SIGCHLD, &sa, 0);
	srand(time(0));

	LogManager::GetLogManager()->SetDebugMode(false);
	LogManager::GetLogManager()->Start(iLogLevel, "log_httpc");

	HttpClient::Init();
	HttpClient client;
	HttpEntiy httpEntiy;
	httpEntiy.SetKeepAlive(true);
	string baseUrl = "http://127.0.0.1/keepalive";

	long long startTime = getCurrentTime();
	for(int i = 0; i < 10000; i++) {
		string url = baseUrl ;
		bool bGet = i % 2;
		httpEntiy.SetGetMethod(bGet);
		if (bGet) {
			url += "?times=" + to_string(i);
		} else {
			httpEntiy.SetRawData("times=" + to_string(i));
		}

		LogAync(
				LOG_NOTICE,
				"req %s %s",
				url.c_str(),
				httpEntiy.GetContentDesc().c_str()
				);

		bool bFlag = client.Request(url, &httpEntiy, false);
		const char* rep = NULL;
		int repSize = 0;
		client.GetBody(&rep, repSize);

		LogAync(
				LOG_NOTICE,
				"rep %d %s",
				client.GetRespondCode(),
				rep
				);

//		sleep(1);
	}
	long long endTime = getCurrentTime();

	LogAync(
			LOG_NOTICE,
			"total time %d ms",
			endTime - startTime
			);

	return EXIT_SUCCESS;
}

void SignalFunc(int signal) {
	switch(signal) {
	case SIGINT:
	case SIGQUIT:
	case SIGTERM: // can handle
	case SIGKILL:
	case SIGBUS:
	case SIGSEGV:{
		LogAyncUnSafe(
				LOG_ALERT, "main( Get Exit Signal, signal : %d )", signal
				);
		LogManager::GetLogManager()->LogFlushMem2File();
		exit(0);
	}break;
	default:{
		LogAync(
				LOG_ALERT, "main( Get signal : %d )", signal
				);
//		LogManager::GetLogManager()->LogFlushMem2File();
	}break;
	}
}

bool Parse(int argc, char *argv[]) {
	string key;
	string value;

	for( int i = 1; i < argc;) {
		key = argv[i++];

		if( key.compare("-v") == 0 ) {
			if (i + 1 < argc) {
				value = argv[i++];
				iLogLevel = atoi(value.c_str());
				iLogLevel = MIN(iLogLevel, LOG_DEBUG);
				iLogLevel = MAX(iLogLevel, LOG_OFF);
			}
		} else if( key.compare("-d") == 0 ) {
			LogManager::GetLogManager()->SetSTDMode(true);
		}
	}

	printf("# Usage: ./httpc -v [LogLevel] \n");
	printf("# Example: ./httpc -v 4 \n");
	printf("# Config: [Log Level: %s]\n", LogManager::LogLevelDesc(iLogLevel).c_str());

	return true;
}
