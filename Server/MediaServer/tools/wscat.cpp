/*
 * wscat.cpp
 *
 *  Created on: 2020-11-03
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
#include <common/Arithmetic.h>
#include <common/StringHandle.h>
#include <common/CommonFunc.h>

#include <mongoose/mongoose.h>

char url[1024] = {"ws://127.0.0.1:9981"};
char cmd[1024] = {""};
int timeout = 10;
bool gRunning = true;
bool gConnected = false;

bool Parse(int argc, char *argv[]);

void Handle(struct mg_connection *nc, int ev, void *ev_data) {
    struct websocket_message *wm = (struct websocket_message *)ev_data;

    switch (ev) {
		case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST:{
		}break;
        case MG_EV_WEBSOCKET_HANDSHAKE_DONE:{
        	gConnected = true;
        	if (strlen(cmd) > 0) {
        		mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, cmd, strlen(cmd));
        	}
        }break;
        case MG_EV_WEBSOCKET_FRAME:{
        	string msg((const char *)wm->data, wm->size);
        	printf("%s", msg.c_str());
        	mg_shutdown(nc);
        }break;
        case MG_EV_CLOSE:{
        	if (!gConnected) {
        		printf("Connect fail \n");
        	}

        	gRunning = false;
        }break;
    }
}

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

	mg_mgr mgr;
	mg_mgr_init(&mgr, NULL);
	struct mg_connect_opts opt = {0};
	mg_connection *conn = mg_connect_ws_opt(&mgr, Handle, opt, url, "", "User-Agent: wscat\r\n");
	if ( NULL != conn && conn->err == 0 ) {
		long long start = getCurrentTime();
		while (gRunning) {
			mg_mgr_poll(&mgr, 100);
			long long now = getCurrentTime();
			if ( (now - start) > timeout * 1000 ) {
				printf("Request timeout");
				break;
			}
		}
	}

	return EXIT_SUCCESS;
}

void SignalFunc(int sign_no) {
	switch(sign_no) {
	default:{
//		signal(sign_no, SIG_DFL);
		exit(0);
	}break;
	}
}

bool Parse(int argc, char *argv[]) {
	string key;
	string value;

	for( int i = 1; (i + 1) < argc; i+=2 ) {
		key = argv[i];
		value = argv[i+1];

		if( key.compare("-c") == 0 ) {
			memset(url, 0, sizeof(url));
			memcpy(url, value.c_str(), value.length());
		} else if( key.compare("-x") == 0 ) {
			memset(cmd, 0, sizeof(cmd));
			memcpy(cmd, value.c_str(), value.length());
		} else if ( key.compare("-t") == 0) {
			timeout = atoi(value.c_str());
		}
	}

	if ((strlen(url) == 0)) {
		printf("# Usage: ./wscat -c [WebsocketHost] -t [Timeout] -x [Cmd] \n");
		printf("# Example: ./wscat -c ws://127.0.0.1:9981 -t 10 -x {\"data\":null,\"errmsg\":\"\",\"errno\":0,\"id\":0,\"route\":\"imRTC/sendPing\"} \n");
	}

	return true;
}
