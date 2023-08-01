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
using namespace qpidnetwork;

string gConfFilePath = "";  // 配置文件
static MediaServer gServer;

bool Parse(int argc, char *argv[]);
void SignalFunc(int sign_no);

int main(int argc, char *argv[]) {
	printf("############## MediaServer ############## \n");
	printf("# Version:%s \n", VERSION_STRING);
	printf("# Build date:%s %s \n", __DATE__, __TIME__);
	srand(time(0));

	// 忽略对已经关闭的Socket发送信息导致错误
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGPIPE, &sa, 0);
	// 忽略在后台时候进行标准输入输出
//	signal(SIGTTIN, SIG_IGN);
//	signal(SIGTTOU, SIG_IGN);

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
		bFlag = gServer.Start(gConfFilePath);
	} else {
		printf("# Usage:./mediaserver [ -f <config file> ] \n");
		bFlag = gServer.Start("/etc/mediaserver.config");
	}

	LogManager::GetLogManager()->LogFlushMem2File();

	if (bFlag) {
		printf("%s", Banner());
	}

	while (bFlag && !gServer.IsNeedStop()) {
		LogManager::GetLogManager()->LogFlushMem2File();
		fflush(stdout);
		sleep(1);
	}

	gServer.Stop();
	LogManager::GetLogManager()->Stop();
	printf("# main() exit \n");

	return EXIT_SUCCESS;
}

bool Parse(int argc, char *argv[]) {
	string key, value;
	for( int i = 1; i < argc;) {
		key = argv[i++];

		if( key.compare("-f") == 0 ) {
			value = argv[i++];
			gConfFilePath = value;
		} else if( key.compare("-d") == 0 ) {
			LogManager::GetLogManager()->SetSTDMode(true);
		}
	}

	return true;
}

void SignalFunc(int sig) {
	switch(sig) {
	case SIGCHLD:{
		int status;
		int pid = 0;
		while (true) {
			int pid = waitpid(-1, &status, WNOHANG);
			if ( pid > 0 ) {
				printf("# main, waitpid, pid:%d \n", pid);
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
				LOG_ALERT, "main, Get Exit Signal, sig:%d", sig
				);
		MainLoop::GetMainLoop()->Exit(SIGKILL);
		gServer.Exit(sig);
		LogManager::GetLogManager()->LogFlushMem2File();
	}break;
	case SIGBUS:
	case SIGABRT:
	case SIGSEGV:{
		LogAyncUnSafe(
				LOG_ALERT, "main, Get Error Signal, sig:%d", sig
				);
		MainLoop::GetMainLoop()->Exit(SIGKILL);
		gServer.Exit(sig);
		LogManager::GetLogManager()->LogFlushMem2File();
		/**
		 * 不能调用exit()
		 * 因为收到sig会进行中断，其他线程可能正在malloc()/free()，进行内核态的锁
		 * 而调用exit()=>__run_exit_handlers()=>OPENSSL_CLEANUP()=>free()，则会导致死锁
		 * exit()会写入缓冲区到内核, _exit()不会
		 */
		_exit(1);
	}break;
	default:{
		LogAyncUnSafe(
				LOG_ALERT, "main, Get Other Signal, sig:%d", sig
				);
		MainLoop::GetMainLoop()->Exit(SIGKILL);
		gServer.Exit(sig);
		LogManager::GetLogManager()->LogFlushMem2File();
	}break;
	}
}
