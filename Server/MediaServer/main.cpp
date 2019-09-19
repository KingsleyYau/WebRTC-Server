/*
 * server.cpp
 *
 *  Created on: 2019-07-21
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
#include <map>
using namespace std;

// Main
#include <server/MainLoop.h>

#include "MediaServer.h"

string gConfFilePath = "";  // 配置文件
static MediaServer gMediaServer;

bool Parse(int argc, char *argv[]);
void SignalFunc(int sign_no);
const char *Banner(void);

int main(int argc, char *argv[]) {
	printf("############## MediaServer ############## \n");
	printf("# Version : %s \n", VERSION_STRING);
	printf("# Build date : %s %s \n", __DATE__, __TIME__);
	srand(time(0));

	// 忽略对已经关闭的Socket发送信息导致错误
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGPIPE, &sa, 0);

	// 不需要重新执行
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SignalFunc;
	sigemptyset(&sa.sa_mask);
	// 回收子进程
	sigaction(SIGCHLD, &sa, 0);

	/* Handle */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SignalFunc;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);

//	sigaction(SIGHUP, &sa, 0);
	// Ctrl-C
	sigaction(SIGINT, &sa, 0);
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

	Parse(argc, argv);

	bool bFlag = false;
	if( gConfFilePath.length() > 0 ) {
		bFlag = gMediaServer.Start(gConfFilePath);
	} else {
		printf("# Usage : ./rtp2rtmp-server [ -f <config file> ] \n");
		bFlag = gMediaServer.Start("/etc/rtp2rtmp-server.config");
	}

	LogManager::GetLogManager()->LogFlushMem2File();

	if (bFlag) {
		printf("%s", Banner());
	}

	while( bFlag && gMediaServer.IsRunning() ) {
		LogManager::GetLogManager()->LogFlushMem2File();
		fflush(stdout);
		sleep(5);
	}

	return EXIT_SUCCESS;
}

bool Parse(int argc, char *argv[]) {
	string key, value;
	for( int i = 1; (i + 1) < argc; i+=2 ) {
		key = argv[i];
		value = argv[i+1];

		if( key.compare("-f") == 0 ) {
			gConfFilePath = value;
		}
	}

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
		signal(sign_no, SIG_DFL);
		kill(getpid(), sign_no);
	}break;
	}
}

const char *Banner(void) {
	return ("\n"
			"\033[33;44m.=============================================================.\033[0m\n"
			"\033[33;44m|   __  ___         ___       _____                           |\033[0m\n"
			"\033[33;44m|   /  |/  /__  ____/ (_)___ _/ ___/___  ______   _____  _____|\033[0m\n"
			"\033[33;44m|  / /|_/ / _ \\/ __  / / __ `/\\__ \\/ _ \\/ ___/ | / / _ \\/ ___/|\033[0m\n"
			"\033[33;44m| / /  / /  __/ /_/ / / /_/ /___/ /  __/ /   | |/ /  __/ /    |\033[0m\n"
			"\033[33;44m|/_/  /_/\\___/\\__,_/_/\\__,_//____/\\___/_/    |___/\\___/_/     |\033[0m\n"
			"\033[33;44m|                                                             |\033[0m\n"
			"\033[33;44m.=============================================================.\033[0m\n"
			"\033[33;44m|   MediaServer (Media Gateway for WebRTC)                    |\033[0m\n"
			"\033[33;44m|   Author: Max.Chiu                                          |\033[0m\n"
			"\033[33;44m|   Email: Kingsleyyau@gmail.com                              |\033[0m\n"
			"\033[33;44m.=============================================================.\033[0m\n"
			);
}
