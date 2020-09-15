/*
 * httpd.cpp
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

// ThirdParty
#include <json/json.h>
// Common
#include <common/LogManager.h>
#include <common/Arithmetic.h>
#include <common/StringHandle.h>
// Base Server
#include <server/AsyncIOServer.h>
// Request/Respond
#include <parser/HttpParser.h>
#include <request/IRequest.h>
#include <respond/IRespond.h>
#include <respond/BaseRespond.h>
#include <respond/BaseRawRespond.h>
#include <respond/BaseResultRespond.h>

using namespace mediaserver;
unsigned int gReqCount = 0;
AsyncIOServer gServer;
class HttpParserCallbackImp : public HttpParserCallback {
public:
	bool HttpSendRespond(
			HttpParser* parser,
			string body
			) {
		bool bFlag = false;
		Client* client = (Client *)parser->custom;

		// 发送头部
		char buffer[MAX_BUFFER_LEN];
		snprintf(
				buffer,
				MAX_BUFFER_LEN - 1,
				"HTTP/1.1 200 OK\r\n"
				"Connection: close\r\n"
//				"Content-Length: %d\r\n"
				"Content-Type: text/html; charset=utf-8\r\n"
				"\r\n"
//				(int)body.size()
				);
		int len = strlen(buffer);
		gServer.Send(client, buffer, len);

		if( !body.empty() ) {
			// 发送内容
			bool more = false;
			while( true ) {
				int send = (int)body.size();
				gServer.Send(client, body.c_str(), send);

				if( !more ) {
					// 全部发送完成
					bFlag = true;
					break;
				}
			}
		}

		if( !bFlag ) {
			LogAync(
					LOG_WARNING,
					"Send( "
					"[Fail], "
					"client : %p\n"
					"%s%s\n"
					")",
					client,
					buffer,
					body.c_str()
					);
		} else {
			LogAync(
					LOG_NOTICE,
					"Send( "
					"client : %p\n"
					"%s%s\n"
					")",
					client,
					buffer,
					body.c_str()
					);
		}

		return bFlag;
	}

	bool HttpParseRequestHeader(HttpParser* parser) {
		bool bFlag = true;

		if ( parser->GetPath() == "/verify/v1/start" ) {
			// 马上返回数据
			HttpSendRespond(parser, "{\"errno\":0,\"errmsg\":\"\"}");
		} else if ( parser->GetPath() == "/verify/v1/login" ) {
			if ( gReqCount++ % 60 == 0 ) {
				int second = rand() % 40;

				LogAync(
						LOG_NOTICE,
						"Recv( "
						"[Sleep %d seconds before response], "
						"%s "
						")",
						second,
						parser->GetRawFirstLine().c_str()
						);

				sleep(second);
			}

			int success = ((gReqCount % 10)!=0);
			char buffer[256] = {0};
			snprintf(buffer, sizeof(buffer), "{\"errno\":0,\"errmsg\":\"\",\"data\":{\"success\":%d,\"userid\":\"max\"}}", success);
			HttpSendRespond(parser, buffer);
		} else if ( parser->GetPath() == "/verify/v1/verifyrtmp" ) {
			int success = ((gReqCount % 30)!=0);
			char buffer[256] = {0};
			snprintf(buffer, sizeof(buffer), "{\"errno\":%d,\"errmsg\":\"\"}}", !success);
			HttpSendRespond(parser, buffer);
		} else if ( parser->GetPath() == "/verify/v1/shutdown" ) {
			int success = ((gReqCount % 50)!=0);
			char buffer[256] = {0};
			snprintf(buffer, sizeof(buffer), "{\"errno\":%d,\"errmsg\":\"\"}}", !success);
			HttpSendRespond(parser, buffer);
		} else if ( parser->GetPath() == "/record/v1/record" ) {
			int success = ((gReqCount % 50)!=0);
			char buffer[256] = {0};
			snprintf(buffer, sizeof(buffer), "{\"errno\":%d,\"errmsg\":\"\"}}", !success);
			HttpSendRespond(parser, buffer);
		} else {
			bFlag = true;
			HttpSendRespond(parser, "{\"errno\":1,\"errmsg\":\"\"}");
		}

		return bFlag;
	}

	void OnHttpParserHeader(HttpParser* parser) {
		Client* client = (Client *)parser->custom;

		LogAync(
				LOG_NOTICE,
				"Recv( "
				"client : %p, "
				"%s "
				")",
				client,
				parser->GetRawFirstLine().c_str()
				);

		bool bFlag = HttpParseRequestHeader(parser);
		// 已经处理则可以断开连接
		if( bFlag ) {
			gServer.Disconnect(client);
		}
	}

	bool HttpParseRequestBody(HttpParser* parser) {
		bool bFlag = true;

		{
			// 未知命令
//			HttpSendRespond(parser, "{\"errno\":1,\"errmsg\":\"\"}}");
		}

		return bFlag;
	}

	void OnHttpParserBody(HttpParser* parser) {
		Client* client = (Client *)parser->custom;

		bool bFlag = HttpParseRequestBody(parser);
		if ( bFlag ) {
			gServer.Disconnect(client);
		}
	}

	void OnHttpParserError(HttpParser* parser) {
		Client* client = (Client *)parser->custom;

		LogAync(
				LOG_WARNING,
				"OnHttpParserError( "
				"client : %p, "
				"%s "
				")",
				parser,
				client,
				parser->GetRawFirstLine().c_str()
				);

		gServer.Disconnect(client);
	}
} gHttpParserCallbackImp;

class AsyncIOServerCallbackImp :public AsyncIOServerCallback {
public:
	bool OnAccept(Client *client) {
		HttpParser* parser = new HttpParser();
		parser->SetCallback(&gHttpParserCallbackImp);
		parser->custom = client;
		client->parser = parser;

		LogAync(
				LOG_INFO,
				"OnAccept( "
				"parser : %p, "
				"client : %p "
				")",
				parser,
				client
				);
		return true;
	}

	void OnDisconnect(Client* client) {
		HttpParser* parser = (HttpParser *)client->parser;

		LogAync(
				LOG_INFO,
				"OnDisconnect( "
				"parser : %p, "
				"client : %p "
				")",
				parser,
				client
				);

		if( parser ) {
			delete parser;
			client->parser = NULL;
		}
	}
} gAsyncIOServerCallbackImp;

void SignalFunc(int sign_no);
int main(int argc, char *argv[]) {
	printf("############## httpd ############## \n");
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

	LogManager::GetLogManager()->SetSTDMode(true);
	LogManager::GetLogManager()->SetDebugMode(false);
	LogManager::GetLogManager()->Start(LOG_NOTICE, "log");

	gServer.SetAsyncIOServerCallback(&gAsyncIOServerCallbackImp);
	bool bFlag = gServer.Start(5555, 10000, 4);
	while( bFlag && gServer.IsRunning() ) {
		LogManager::GetLogManager()->LogFlushMem2File();
		fflush(stdout);
		sleep(5);
	}

	return EXIT_SUCCESS;
}

void SignalFunc(int sign_no) {
	switch(sign_no) {
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
