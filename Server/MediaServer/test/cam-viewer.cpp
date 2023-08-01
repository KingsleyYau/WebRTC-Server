/*
 * dbtest.cpp
 *
 *  Created on: 2015-1-14
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

// Common
#include <common/LogManager.h>
#include <common/KThread.h>

#include "CamViewer.h"
using namespace qpidnetwork;

char ws_host[128] = {"192.168.88.133:8080"};
char interface[128] = {""};//{"192.168.88.134"};
char name[128] = {"MM"};
char dest[128] = {"WW"};
int iCurrent = 0;
int iTotal = 1;
int iReconnect = 0;
LOG_LEVEL iLogLevel = LOG_NOTICE;

static CamViewer gTester;

bool Parse(int argc, char *argv[]);
void SignalFunc(int sign_no);

int main(int argc, char *argv[]) {
	printf("############## cam-viewer ############## \n");
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

	LogManager::GetLogManager()->Start(iLogLevel, "./log");
	LogManager::GetLogManager()->LogSetFlushBuffer(1 * BUFFER_SIZE_1K * BUFFER_SIZE_1K);

    string baseUrl = string(ws_host);
    bool bFlag = gTester.Start(name, dest, baseUrl, iTotal, iReconnect);

	while( bFlag && gTester.IsRunning() ) {
		/* do nothing here */
		fflush(stdout);
		sleep(3);
	}

	gTester.Stop();
	LogManager::GetLogManager()->Stop();

	printf("# cam-viewer exit \n");
	return EXIT_SUCCESS;
}

bool Parse(int argc, char *argv[]) {
	string key;
	string value;

	for(int i = 1; i < argc;) {
		key = argv[i++];

		if( key.compare("-ws") == 0 ) {
			value = argv[i++];
			memset(ws_host, 0, sizeof(ws_host));
			memcpy(ws_host, value.c_str(), value.length());
		} else if( key.compare("-name") == 0 ) {
			value = argv[i++];
			memset(name, 0, sizeof(name));
			memcpy(name, value.c_str(), value.length());
		} else if( key.compare("-dest") == 0 ) {
			value = argv[i++];
			memset(dest, 0, sizeof(dest));
			memcpy(dest, value.c_str(), value.length());
		} else if( key.compare("-i") == 0 ) {
			value = argv[i++];
			memset(interface, 0, sizeof(interface));
			memcpy(interface, value.c_str(), value.length());
		} else if( key.compare("-n") == 0 ) {
			value = argv[i++];
			iTotal = atoi(value.c_str());
		} else if( key.compare("-r") == 0 ) {
			value = argv[i++];
			iReconnect = atoi(value.c_str());
		} else if ( key.compare("-v") == 0 ) {
			value = argv[i++];
			iLogLevel = atoi(value.c_str());
			iLogLevel = MIN(iLogLevel, LOG_DEBUG);
			iLogLevel = MAX(iLogLevel, LOG_OFF);
		} else if( key.compare("-d") == 0 ) {
			LogManager::GetLogManager()->SetSTDMode(true);
		} else {
			printf("# Usage: ./cam-viewer -ws [WebsocketHost] -name [Name] -dest [Dest] -i [LocalIp] -n [Count] -r [Reconnect] -v [LogLevel, 0-6] -d [Std Log]\n");
			printf("# Example: ./cam-viewer -ws 192.168.88.133:8080 -name MM -dest WW -i 192.168.88.134 -n 1 -r 60 -v 4 \n");
			return false;
		}
	}

	printf("# Config: [ws : %s], [name : %s], [dest : %s], [interface : %s], [iTotal : %d], [iReconnect : %d], [Log Level: %s] \n", ws_host, name, dest, interface, iTotal, iReconnect, LogManager::LogLevelDesc(iLogLevel).c_str());

	return true;
}

void SignalFunc(int signal) {
	switch(signal) {
	case SIGCHLD:{
		int status;
		int pid = 0;
		while (true) {
			int pid = waitpid(-1, &status, WNOHANG);
			if ( pid > 0 ) {
				printf("# main( waitpid:%d ) \n", pid);
			} else {
				break;
			}
		}
	}break;
	case SIGINT:
	case SIGQUIT:
	case SIGTERM:{
		LogAyncUnSafe(
				LOG_ALERT, "main( Get Exit Signal, signal:%d )", signal
				);
		MainLoop::GetMainLoop()->Exit(SIGKILL);
		gTester.Exit(signal);
		LogManager::GetLogManager()->LogFlushMem2File();
	}break;
	case SIGBUS:
	case SIGABRT:
	case SIGSEGV:{
		LogAyncUnSafe(
				LOG_ALERT, "main( Get Error Signal, signal:%d )", signal
				);
		gTester.Exit(signal);
		LogManager::GetLogManager()->LogFlushMem2File();
		_exit(1);
	}break;
	default:{
		LogAyncUnSafe(
				LOG_ALERT, "main( Get Other Signal, signal:%d )", signal
				);
		gTester.Exit(signal);
		LogManager::GetLogManager()->LogFlushMem2File();
		_exit(1);
	}break;
	}
}
