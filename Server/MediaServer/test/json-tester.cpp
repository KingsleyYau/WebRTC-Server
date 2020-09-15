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

// ThirdParty
#include <json/json.h>
// Common
#include <common/LogManager.h>
#include <common/Arithmetic.h>
#include <common/StringHandle.h>

bool Parse(int argc, char *argv[]);
void SignalFunc(int sign_no);
string SqlTransferInsert(const string& sql);

class A {
public:
	virtual ~A(){ printf("# ~A \n"); };
};
class B : public A {
public:
	B(){ printf("# B \n"); };
};
int main(int argc, char *argv[]) {
	printf("############## json-test ############## \n");
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

	string input = "{\"http-flv\":{\"nginx_version\":\"1.11.13\",\"nginx_http_flv_version\":\"1.2.5\",\"compiler\":\"gcc 4.8.5 20150623 (Red Hat 4.8.5-39) (GCC) \",\"built\":\"May 29 2020 17:21:04\",\"pid\":5930,\"uptime\":692441,\"naccepted\":1800,\"bw_in\":1165768,\"bytes_in\":28982579394,\"bw_out\":1114160,\"bytes_out\":23230814460,\"servers\":[[{\"name\":\"cdn_flash\",\"live\":{\"streams\":[],\"nclients\":0}},{\"name\":\"cdn_standard_no_auth\",\"live\":{\"streams\":[],\"nclients\":0}},{\"name\":\"cdn_standard\",\"live\":{\"streams\":[],\"nclients\":0}},{\"name\":\"publish_standard_allframe\",\"live\":{\"streams\":[],\"nclients\":0}},{\"name\":\"cdn_standard_1\",\"live\":{\"streams\":[],\"nclients\":0}},{\"name\":\"cdn_standard_2\",\"live\":{\"streams\":[],\"nclients\":0}}],[{\"name\":\"publish_flash\",\"live\":{\"streams\":[],\"nclients\":0}},{\"name\":\"publish_flash_allframe\",\"live\":{\"streams\":[],\"nclients\":0}},{\"name\":\"publish_standard\",\"live\":{\"streams\":[],\"nclients\":0}},{\"name\":\"publish_standard_allframe\",\"live\":{\"streams\":[],\"nclients\":0}},{\"name\":\"play_flash\",\"live\":{\"streams\":[],\"nclients\":0}},{\"name\":\"play_standard\",\"live\":{\"streams\":[],\"nclients\":0}},{\"name\":\"play_flash_1\",\"live\":{\"streams\":[],\"nclients\":0}},{\"name\":\"play_standard_1\",\"live\":{\"streams\":[{\"name\":\"anchor_A488416_338240\",\"time\":678889287,\"bw_in\":0,\"bytes_in\":0,\"bw_out\":0,\"bytes_out\":0,\"bw_audio\":0,\"bw_video\":0,\"clients\":[],\"nclients\":0,\"publishing\":false,\"active\":false}{\"name\":\"anchor_B798735_340330\",\"time\":688783683,\"bw_in\":0,\"bytes_in\":0,\"bw_out\":0,\"bytes_out\":0,\"bw_audio\":0,\"bw_video\":0,\"clients\":[],\"nclients\":0,\"publishing\":false,\"active\":false}{\"name\":\"anchor_G458108_348645\",\"time\":29962,\"bw_in\":1114392,\"bytes_in\":3580942,\"bw_out\":1114392,\"bytes_out\":3333139,\"bw_audio\":106384,\"bw_video\":1008008,\"clients\":[{\"id\":30153,\"address\":\"\",\"time\":29962,\"flashver\":\"ngx-local-relay\",\"dropped\":0,\"avsync\":60,\"timestamp\":338470,\"publishing\":true,\"active\":true},{\"id\":30150,\"address\":\"103.29.140.19\",\"time\":34371,\"flashver\":\"WIN 15,0,0,239\",\"dropped\":15,\"avsync\":60,\"timestamp\":338470,\"publishing\":false,\"active\":true}],\"meta\":{\"video\":{\"width\":640,\"height\":360,\"frame_rate\":15,\"codec\":\"H264\",\"profile\":\"Main\",\"level\":\"5.1\"}, \"audio\": {\"channels\":\"1\",\"sample_rate\":44100,\"profile\":\"LC\"}},\"nclients\":2,\"publishing\":true,\"active\":true}],\"nclients\":2}},{\"name\":\"play_flash_2\",\"live\":{\"streams\":[],\"nclients\":0}},{\"name\":\"play_standard_2\",\"live\":{\"streams\":[],\"nclients\":0}}]]}}";
//	input = "{\"streams\":[{\"active\":1}{\"active\":2}{\"active\":3},{\"active\":4}]}";
	//	Json::FastWriter writer;
//	Json::Value jsonMsg;
//	jsonMsg["anchorId"] = "123";
//	string msg = writer.write(jsonMsg);
//	printf("# Json Encode msg OK, msg: %s \n", msg.c_str());
//
//	Json::Value jsonRecord;
//	jsonRecord["msg"] = msg;
//	string record = writer.write(jsonRecord);
//	printf("# Json Encode record OK, record: %s \n", record.c_str());
//
//	Json::Value jsonArray;
//	jsonArray.append(record);
//	Json::Value jsonDatalist;
//	jsonDatalist["datalist"] = jsonArray;
//	string output = writer.write(jsonDatalist);
//	printf("# Json Encode OK, output: %s \n", output.c_str());

	const void* p = NULL;
	printf("# p: %p \n", p);

	Json::Reader reader;
	Json::Value reqRoot;
	bool bParse = reader.parse(input, reqRoot, false);
	if ( bParse ) {
		printf("# Json Parse OK, input: %s \n\n", input.c_str());
		if ( reqRoot.isObject() ) {
			Json::Value streams = reqRoot["streams"];
			printf("# streams.size: %d \n", (int)streams.size());
			for(int j = 0; j < (int)streams.size(); j++) {
				Json::Value stream = streams[j];
				if ( stream.isObject() && stream["active"].isInt() ) {
					printf("# %d, active: %d \n", j, stream["active"].asInt());
				} else {
					printf("# %d, active: error \n", j);
				}
			}
		}
//		if ( reqRoot.isObject() && reqRoot["http-flv"]["servers"].isArray() && reqRoot["http-flv"]["servers"].size() > 0 ) {
//			Json::Value apps = reqRoot["http-flv"]["servers"][1];
//			printf("# apps.size: %d \n", (int)apps.size());
//			for(int i = 0; i < (int)apps.size(); i++) {
//				Json::Value app = apps[i];
//				Json::Value streams = app["live"]["streams"];
//				printf("# %d, streams.size: %d \n", i, (int)streams.size());
//				for(int j = 0; j < (int)streams.size(); j++) {
//					Json::Value stream = streams[j];
//					if ( stream.isObject() ) {
//						Json::Value clients = stream["clients"];
//						string name = stream["name"].asString();
//						printf("# %d, stream : %s, clients.size: %d \n", j, name.c_str(), (int)clients.size());
//						for(int k = 0; k < (int)clients.size(); k++) {
//							Json::Value client = clients[k];
//							if ( client.isObject() ) {
//								int iClientId = client["id"].asInt();
//								printf("# %d, iClientId: %d \n", k, iClientId);
//								if ( iClientId > 0 ) {
//									char clientId[256] = {0};
//									snprintf(clientId, sizeof(clientId) - 1, "%u", iClientId);
//								}
//							}
//						}
//					}
//				}
//			}
//		}
//		Arithmetic ari;
//		string inputHex = ari.AsciiToHexWithSep((char *)info.c_str(), info.length());
//		printf("# Json Read OK, inputHex: %s \n\n", inputHex.c_str());

//		string outputHex = ari.AsciiToHexWithSep((char *)output.c_str(), output.length());
//		printf("# Write OK, outputHex: %s \n\n", outputHex.c_str());
	} else {
		printf("# Json Parse Fail, input: %s \n\n", input.c_str());
	}

	return EXIT_SUCCESS;
}

bool Parse(int argc, char *argv[]) {
	string key;
	string value;

	for( int i = 1; (i + 1) < argc; i+=2 ) {
		key = argv[i];
		value = argv[i+1];
	}

	return true;
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

string SqlTransferInsert(const string& sql) {
	string result = "";

	result = StringHandle::replace(sql, "\\", "\\\\");
	result = StringHandle::replace(result, "'", "\\'");

	return result;
}
