/*
 * wsserver_test.cpp
 *
 *  Created on: 2021/02/18
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
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

#include <websocket/WSServer.h>
using namespace mediaserver;

class WSServerCallbackImp : public WSServerCallback {
public:
	void OnWSOpen(WSServer *server, connection_hdl hdl, const string& addr, const string& userAgent) {
//		server->Disconnect(hdl);
	}
	void OnWSClose(WSServer *server, connection_hdl hdl) {

	}
	void OnWSMessage(WSServer *server, connection_hdl hdl, const string& str) {
//		server->SendText(hdl, "123");
		printf("# %p OnWSMessage \n", hdl.lock().get());
		server->Disconnect(hdl);
	}
};
WSServerCallbackImp gCallback;

bool Parse(int argc, char *argv[]);
void SignalFunc(int sign_no);

int main(int argc, char *argv[]) {
	srand(time(0));
	Parse(argc, argv);

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

	WSServer server;
	server.SetCallback(&gCallback);
	server.Start(9981, 1000);
	while(true) {
		sleep(1);
	}

	return EXIT_SUCCESS;
}

bool Parse(int argc, char *argv[]) {
	string key, value;
	for( int i = 1; (i + 1) < argc; i+=2 ) {
		key = argv[i];
		value = argv[i+1];

		if( key.compare("-f") == 0 ) {
		}
	}

	return true;
}

void SignalFunc(int sign_no) {
	switch(sign_no) {
	default:{
		exit(0);
	}break;
	}
}
