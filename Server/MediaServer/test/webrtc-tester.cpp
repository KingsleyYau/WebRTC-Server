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

#include "WebRTCTester.h"
using namespace mediaserver;

// Common
#include <common/LogManager.h>

char ws_host[128] = {"192.168.88.133:9981"};
char turn[128] = {"192.168.88.133"};
char interface[128] = {""};//{"192.168.88.134"};
char name[128] = {"tester"};
int iCurrent = 0;
int iTotal = 1;
int iReconnect = 0;

static WebRTCTester gTester;

bool Parse(int argc, char *argv[]);
void SignalFunc(int sign_no);

int main(int argc, char *argv[]) {
	printf("############## webrtc-test ############## \n");
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

	LogManager::GetLogManager()->Start(LOG_INFO, "./log");
	LogManager::GetLogManager()->SetDebugMode(false);
	LogManager::GetLogManager()->LogSetFlushBuffer(1 * BUFFER_SIZE_1K * BUFFER_SIZE_1K);

	WebRTCClient::GobalInit("./ssl/tester.crt", "./ssl/tester.key", turn, interface);

    string baseUrl = "ws://" + string(ws_host);
    bool bFlag = gTester.Start(name, baseUrl, iTotal, turn, iReconnect);

	while( bFlag && gTester.IsRunning() ) {
		/* do nothing here */
		fflush(stdout);
		sleep(3);
	}

	return EXIT_SUCCESS;
}

bool Parse(int argc, char *argv[]) {
	string key;
	string value;

	for( int i = 1; (i + 1) < argc; i+=2 ) {
		key = argv[i];
		value = argv[i+1];

		if( key.compare("-ws") == 0 ) {
			memset(ws_host, 0, sizeof(ws_host));
			memcpy(ws_host, value.c_str(), value.length());
		} else if( key.compare("-name") == 0 ) {
			memset(name, 0, sizeof(name));
			memcpy(name, value.c_str(), value.length());
		} else if( key.compare("-turn") == 0 ) {
			memset(turn, 0, sizeof(turn));
			memcpy(turn, value.c_str(), value.length());
		} else if( key.compare("-i") == 0 ) {
			memset(interface, 0, sizeof(interface));
			memcpy(interface, value.c_str(), value.length());
		} else if( key.compare("-n") == 0 ) {
			iTotal = atoi(value.c_str());
		} else if( key.compare("-r") == 0 ) {
			iReconnect = atoi(value.c_str());
		}
	}

	printf("# Usage: ./webrtc-tester -ws [WebsocketHost] -turn [TurnHost]  -name [Name] -i [LocalIp] -n [Count] -r [Reconnect] \n");
	printf("# Example: ./webrtc-tester -ws 192.168.88.133:9981 -turn 192.168.88.133 -name tester -i 192.168.88.134 -n 1 -r 60 \n");
	printf("# Config: [ws : %s], [turn : %s], [name : %s], [interface : %s], [iTotal : %d], [iReconnect : %d]\n", ws_host, turn, name, interface, iTotal, iReconnect);

	return true;
}

void SignalFunc(int sign_no) {
	switch(sign_no) {
	case SIGCHLD:{
		int status;
		int pid = 0;
		while (true) {
			int pid = waitpid(-1, &status, WNOHANG);
			if ( pid > 0 ) {
				printf("# main( waitpid : %d ) \n", pid);
				MainLoop::GetMainLoop()->Call(pid);
			} else {
				break;
			}
		}
	}break;
	default:{
		LogAyncUnSafe(
				LOG_ALERT, "main( Get signal : %d )", sign_no
				);
		MainLoop::GetMainLoop()->Exit(SIGTERM);
		LogManager::GetLogManager()->LogFlushMem2File();
		exit(0);
	}break;
	}
}
