/*
 * camchat-pusher.cpp
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

#include "CamChatPusher.h"
using namespace qpidnetwork;

// Common
#include <common/LogManager.h>
#include <include/CommonHeader.h>

char ws_host[128] = {"wss://192.168.88.133:13583"};
char turn[128] = {"192.168.88.133"};
char interface[128] = {""};//{"192.168.88.134"};
char name[128] = {"WW"};
char usersFileName[1024] = {""};
char logDir[128] = {"log"};
int iCurrent = 0;
int iTotal = 1;
int iReconnect = 0;
double dPushRatio = 1;
int iLogLevel = LOG_INFO;
bool bTcpForce = false;

static CamChatPusher gTester;

bool Parse(int argc, char *argv[]);
void SignalFunc(int sign_no);

int main(int argc, char *argv[]) {
	printf("############## camchat-pusher ############## \n");
	Parse(argc, argv);
	srand(time(0));

	/* Ignore */
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGPIPE, &sa, 0);
	sigaction(SIGTTOU, &sa, 0);

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

	LogManager::GetLogManager()->Start(iLogLevel, logDir);
//	LogManager::GetLogManager()->SetDebugMode(false);
	LogManager::GetLogManager()->LogSetFlushBuffer(1 * BUFFER_SIZE_1K * BUFFER_SIZE_1K);

	WebRTCClient::GobalInit("./ssl/tester.crt", "./ssl/tester.key", turn, interface);

    string baseUrl = string(ws_host);
    bool bFlag = gTester.Start(name, baseUrl, usersFileName, iTotal, turn, iReconnect, dPushRatio, bTcpForce);

	while( bFlag && gTester.IsRunning() ) {
		/* do nothing here */
		fflush(stdout);
		sleep(3);
	}

	gTester.Stop();
	LogManager::GetLogManager()->Stop();

	printf("# camchat-pusher exit \n");
	return EXIT_SUCCESS;
}

bool Parse(int argc, char *argv[]) {
	string key;
	string value;

	for(int i = 1; i < argc;) {
		key = argv[i++];

		if(key.compare("-ws") == 0) {
			if (i < argc) {
				value = argv[i++];
				memset(ws_host, 0, sizeof(ws_host));
				memcpy(ws_host, value.c_str(), value.length());
			}
		} else if( key.compare("-name") == 0 ) {
			if (i < argc) {
				value = argv[i++];
				memset(name, 0, sizeof(name));
				memcpy(name, value.c_str(), value.length());
			}
		} else if( key.compare("-turn") == 0 ) {
			if (i < argc) {
				value = argv[i++];
				memset(turn, 0, sizeof(turn));
				memcpy(turn, value.c_str(), value.length());
			}
		} else if( key.compare("-log") == 0 ) {
			if (i < argc) {
				value = argv[i++];
				memset(logDir, 0, sizeof(logDir));
				memcpy(logDir, value.c_str(), value.length());
			}
		}  else if( key.compare("-i") == 0 ) {
			if (i < argc) {
				value = argv[i++];
				memset(interface, 0, sizeof(interface));
				memcpy(interface, value.c_str(), value.length());
			}
		} else if( key.compare("-n") == 0 ) {
			if (i < argc) {
				value = argv[i++];
				iTotal = atoi(value.c_str());
			}
		} else if( key.compare("-r") == 0 ) {
			value = argv[i++];
			iReconnect = atoi(value.c_str());
		} else if( key.compare("-pr") == 0 ) {
			if (i < argc) {
				value = argv[i++];
				dPushRatio = atof(value.c_str());
				dPushRatio = MIN(1.0, dPushRatio);
				dPushRatio = MAX(0, dPushRatio);
			}
		} else if( key.compare("-v") == 0 ) {
			if (i < argc) {
				value = argv[i++];
				iLogLevel = atoi(value.c_str());
				iLogLevel = MIN(iLogLevel, LOG_DEBUG);
				iLogLevel = MAX(iLogLevel, LOG_OFF);
			}
		}  else if( key.compare("-f") == 0 ) {
			if (i < argc) {
				value = argv[i++];
				memset(usersFileName, 0, sizeof(usersFileName));
				memcpy(usersFileName, value.c_str(), value.length());
			}
		} else if( key.compare("-t") == 0 ) {
			bTcpForce = true;
		} else if( key.compare("-d") == 0 ) {
			LogManager::GetLogManager()->SetSTDMode(true);
		} else if( key.compare("-ice") == 0 ) {
			LogManager::GetLogManager()->SetDebugMode(true);
			IceClient::EnableDebugLog(true);
		} else {
			printf("# Usage: ./camchat-pusher -ws [WebsocketHost] -turn [TurnHost] -name [Name] -log [LogDir] -i [LocalIp] -n [Count] -r [Reconnect] -pr [Push Ratio(0.0-1.0)] -t [Tcp Turn Force] -v [LogLevel] -d [Std Log]\n");
			printf("# Example: ./camchat-pusher -ws ws://192.168.88.133:13583 -turn 192.168.88.133 -name tester -log log -i 192.168.88.134 -n 1 -r 60 -pr 1.0 -t -v 4 -d \n");
			return false;
		}
	}


	printf("# Config: [ws: %s], [turn: %s], [name: %s], [logDir: %s], [interface: %s], [iTotal: %d], [iReconnect: %d], [Push Ratio: %.2f], [Tcp Turn Force: %s], [Log Level: %s]\n",
			ws_host, turn, name, logDir, interface, iTotal, iReconnect, dPushRatio, BOOL_2_STRING(bTcpForce), LogManager::LogLevelDesc(iLogLevel).c_str());

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
				LogAyncUnSafe(
						LOG_INFO, "main, waitpid:%d", pid
						);
				MainLoop::GetMainLoop()->WaitPid(pid);
			} else {
				break;
			}
		}
	}break;
	case SIGINT:
	case SIGQUIT:
	case SIGTERM:{
		LogAyncUnSafe(
				LOG_ALERT, "main, Get Exit Signal, signal:%d", signal
				);
		MainLoop::GetMainLoop()->Exit(SIGKILL);
		gTester.Exit(signal);
		LogManager::GetLogManager()->LogFlushMem2File();
	}break;
	case SIGBUS:
	case SIGABRT:
	case SIGSEGV:{
		LogAyncUnSafe(
				LOG_ALERT, "main, Get Error Signal, signal:%d", signal
				);
		MainLoop::GetMainLoop()->Exit(SIGKILL);
		gTester.Exit(signal);
		LogManager::GetLogManager()->LogFlushMem2File();
		_exit(1);
	}break;
	default:{
		LogAyncUnSafe(
				LOG_ALERT, "main, Get Other Signal, signal:%d", signal
				);
		MainLoop::GetMainLoop()->Exit(SIGKILL);
		gTester.Exit(signal);
		LogManager::GetLogManager()->LogFlushMem2File();
		_exit(1);
	}break;
	}
}
