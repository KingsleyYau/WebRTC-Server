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

char ws[128] = {"192.168.88.133:9881"};
char turn[128] = {"192.168.88.134"};
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

	LogManager::GetLogManager()->Start(LOG_WARNING, "./log");
	LogManager::GetLogManager()->SetDebugMode(true);
	LogManager::GetLogManager()->LogSetFlushBuffer(1 * BUFFER_SIZE_1K * BUFFER_SIZE_1K);

	WebRTC::GobalInit("./ssl/tester.crt", "./ssl/tester.key", turn, "");

    string baseUrl = "ws://" + string(ws);
    gTester.Start(name, baseUrl, iTotal);

	return EXIT_SUCCESS;
}

bool Parse(int argc, char *argv[]) {
	string key;
	string value;

	for( int i = 1; (i + 1) < argc; i+=2 ) {
		key = argv[i];
		value = argv[i+1];

		if( key.compare("-h") == 0 ) {
			memset(ws, 0, sizeof(ws));
			memcpy(ws, value.c_str(), value.length());
		} else if( key.compare("-name") == 0 ) {
			memset(name, 0, sizeof(name));
			memcpy(name, value.c_str(), value.length());
		} else if( key.compare("-s") == 0 ) {
			memset(turn, 0, sizeof(turn));
			memcpy(turn, value.c_str(), value.length());
		} else if( key.compare("-n") == 0 ) {
			iTotal = atoi(value.c_str());
		} else if( key.compare("-r") == 0 ) {
			iReconnect = atoi(value.c_str());
		}
	}

	printf("# [ws : %s], [turn : %s], [name : %s], [iTotal : %d], [iReconnect : %d]\n", ws, turn, name, iTotal, iReconnect);

	return true;
}

void SignalFunc(int sign_no) {
	switch(sign_no) {
	case SIGCHLD:{
		int status;
		int pid = waitpid(-1, &status, WNOHANG);
		LogAync(
				LOG_MSG, "main( waitpid : %d )", pid
				);
		MainLoop::GetMainLoop()->Call(pid);
	}break;
	default:{
		LogAync(
				LOG_ERR_SYS, "main( Get signal : %d )", sign_no
				);
		LogManager::GetLogManager()->LogFlushMem2File();
		gTester.Stop();
		LogManager::GetLogManager()->LogFlushMem2File();
		signal(sign_no, SIG_DFL);
		kill(getpid(), sign_no);
	}break;
	}
}
