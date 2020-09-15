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

#include "mongoose.h"
#include "CamTester.h"
using namespace mediaserver;

char ws[128] = {"192.168.88.133:8080"};
char interface[128] = {""};//{"192.168.88.134"};
char name[128] = {"tester"};
int iCurrent = 0;
int iTotal = 1;
int iReconnect = 0;

static CamTester gTester;

bool Parse(int argc, char *argv[]);
void SignalFunc(int sign_no);

int main(int argc, char *argv[]) {
	printf("############## cam-test ############## \n");
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

	LogManager::GetLogManager()->Start(LOG_NOTICE, "./log");
	LogManager::GetLogManager()->SetDebugMode(false);
	LogManager::GetLogManager()->LogSetFlushBuffer(1 * BUFFER_SIZE_1K * BUFFER_SIZE_1K);

    string baseUrl = "ws://" + string(ws);
    bool bFlag = gTester.Start(name, baseUrl, iTotal, iReconnect);

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
			memset(ws, 0, sizeof(ws));
			memcpy(ws, value.c_str(), value.length());
		} else if( key.compare("-name") == 0 ) {
			memset(name, 0, sizeof(name));
			memcpy(name, value.c_str(), value.length());
		} else if( key.compare("-i") == 0 ) {
			memset(interface, 0, sizeof(interface));
			memcpy(interface, value.c_str(), value.length());
		} else if( key.compare("-n") == 0 ) {
			iTotal = atoi(value.c_str());
		} else if( key.compare("-r") == 0 ) {
			iReconnect = atoi(value.c_str());
		}
	}

	printf("# Usage: ./webrtc-tester -ws [WebsocketHost] -name [Name] -i [LocalIp] -n [Count] -r [Reconnect] \n");
	printf("# Example: ./webrtc-tester -ws 192.168.88.133:8080 -name tester -i 192.168.88.134 -n 1 -r 60 \n");
	printf("# Config: [ws : %s], [name : %s], [interface : %s], [iTotal : %d], [iReconnect : %d]\n", ws, name, interface, iTotal, iReconnect);

	return true;
}

void SignalFunc(int sign_no) {
	switch(sign_no) {
	case SIGCHLD:{
		int status;
		int pid = waitpid(-1, &status, WNOHANG);
		LogAync(
				LOG_INFO, "main( waitpid : %d )", pid
				);
	}break;
	default:{
		LogAync(
				LOG_ALERT, "main( Get signal : %d )", sign_no
				);
		LogManager::GetLogManager()->LogFlushMem2File();
		signal(sign_no, SIG_DFL);
		kill(getpid(), sign_no);
	}break;
	}
}
