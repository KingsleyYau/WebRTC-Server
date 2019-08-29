/*
 * WebRTCTester.cpp
 *
 *  Created on: 2019/08/21
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "WebRTCTester.h"
// ThirdParty
#include <json/json.h>
#include <common/CommonFunc.h>

namespace mediaserver {

void Tester::Handle(struct mg_connection *nc, int ev, void *ev_data) {
    struct websocket_message *wm = (struct websocket_message *)ev_data;
    Tester *tester = (Tester *)nc->user_data;

    switch (ev) {
		case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST:{
        	LogAync(
        			LOG_MSG,
        			"Tester::Handle( "
					"this : %p, "
					"[WS Handshake Request] "
        			")",
					tester
        			);
		}break;
        case MG_EV_WEBSOCKET_HANDSHAKE_DONE:{
        	LogAync(
        			LOG_WARNING,
        			"Tester::Handle( "
					"this : %p, "
					"[WS Handshake Done], "
					"index : %d "
        			")",
					tester,
					tester->index
        			);
        	tester->rtc.Start("");

        }break;
        case MG_EV_WEBSOCKET_FRAME:{
            // Receive Data
        	string str((const char*)wm->data, wm->size);
        	LogAync(
        			LOG_MSG,
        			"Tester::Handle( "
					"this : %p, "
					"[WS Recv Data], "
					"size : %d,"
					"\r\n%s\r\n"
        			")",
					tester,
					wm->size,
					str.c_str()
        			);
        	tester->HandleRecvData(wm->data, wm->size);

        }break;
        case MG_EV_CLOSE:{
            // Disconnect
        	LogAync(
        			LOG_WARNING,
        			"Tester::Handle( "
					"this : %p, "
					"[WS Close] "
					"index : %d "
        			")",
					tester,
					tester->index
        			);

        	tester->rtc.Stop();
        	tester->Start();

        }break;
    }
}

Tester::Tester() : mMutex(KMutex::MutexType_Recursive) {
	mgr = NULL;
	conn = NULL;
	index = 0;
}

Tester::~Tester() {

}

bool Tester::Init(mg_mgr *mgr, const string& url, const string& stream, int index) {
	this->mgr = mgr;
	this->url = url;
	this->stream = stream;
	this->index = index;

	rtc.Init("./push.sh", "127.0.0.1", 10000 + index);
	rtc.SetCallback(this);

	return true;
}

bool Tester::Start() {
    bool bFlag = false;

	LogAync(
			LOG_MSG,
			"Tester::Start( "
			"this : %p, "
			"url : %s, "
			"index : %d "
			")",
			this,
			url.c_str(),
			index
			);

    if ( url.length() > 0 ) {
		struct mg_connect_opts opt = {0};
		opt.user_data = (void *)this;

		mMutex.lock();
		conn = mg_connect_ws_opt(mgr, Handle, opt, url.c_str(), "", NULL);
		if ( NULL != conn && conn->err == 0 ) {
			bFlag = true;
		}
		mMutex.unlock();
    }

	LogAync(
			LOG_WARNING,
			"Tester::Start( "
			"this : %p, "
			"[%s], "
			"url : %s, "
			"index : %d "
			")",
			this,
			bFlag?"OK":"Fail",
			url.c_str(),
			index
			);

    return bFlag;
}

void Tester::Disconnect() {
	LogAync(
			LOG_WARNING,
			"Tester::Disconnect( "
			"this : %p, "
			"url : %s, "
			"index : %d "
			")",
			this,
			url.c_str(),
			index
			);

	mMutex.lock();
	if ( conn ) {
		mg_shutdown(conn);
		conn = NULL;
	}
	mMutex.unlock();
}

bool Tester::HandleRecvData(unsigned char *data, size_t size) {
	bool bFlag = false;
	Json::Value resRoot;
	Json::Reader reader;

	string str((const char*)data, size);
	bool bParse = reader.parse(str, resRoot, false);
	if ( bParse ) {
		bFlag = true;
		if( resRoot.isObject() ) {
			string route = "";
			if ( resRoot["route"].isString() ) {
				route = resRoot["route"].asString();
			}

			if ( route == "imRTC/sendSdpAnswerNotice" ) {
				if ( resRoot["req_data"].isObject() ) {
					Json::Value resData = resRoot["req_data"];

					string sdp = "";
					if ( resData["sdp"].isString() ) {
						sdp = resData["sdp"].asString();
						rtc.UpdateCandidate(sdp);
					}
				}
			}
		}
	}

	return bFlag;
}

void Tester::OnWebRTCServerSdp(WebRTC *rtc, const string& sdp) {
//	LogAync(
//			LOG_WARNING,
//			"Tester::OnWebRTCServerSdp( "
//			"this : %p, "
//			"rtc : %p, "
//			"sdp:\n%s"
//			")",
//			this,
//			rtc,
//			sdp.c_str()
//			);

	Json::Value reqRoot;
	Json::Value reqData = Json::Value::null;
	Json::FastWriter writer;

	reqRoot["id"] = "0";
	reqRoot["route"] = "imRTC/sendSdpCall";

	reqData["sdp"] = sdp;
	reqData["stream"] = stream;
	reqRoot["req_data"] = reqData;

	string req = writer.write(reqRoot);

	mMutex.lock();
	if( conn ) {
		mg_send_websocket_frame(conn, WEBSOCKET_OP_TEXT, (const void *)req.c_str(), req.length());
	}
	mMutex.unlock();
}

void Tester::OnWebRTCStartMedia(WebRTC *rtc) {
	LogAync(
			LOG_WARNING,
			"Tester::OnWebRTCStartMedia( "
			"this : %p, "
			"rtc : %p, "
			"index : %d "
			")",
			this,
			rtc,
			index
			);
}

void Tester::OnWebRTCError(WebRTC *rtc, WebRTCErrorType errType, const string& errMsg) {
	LogAync(
			LOG_WARNING,
			"Tester::OnWebRTCError( "
			"this : %p, "
			"rtc : %p, "
			"index : %d "
			")",
			this,
			rtc,
			index
			);

	mMutex.lock();
	if ( conn ) {
		mg_shutdown(conn);
		conn = NULL;
	}
	mMutex.unlock();
}

void Tester::OnWebRTCClose(WebRTC *rtc) {
	LogAync(
			LOG_WARNING,
			"Tester::OnWebRTCClose( "
			"this : %p, "
			"rtc : %p, "
			"index : %d "
			")",
			this,
			rtc,
			index
			);

	mMutex.lock();
	if ( conn ) {
		mg_shutdown(conn);
		conn = NULL;
	}
	mMutex.unlock();
}

WebRTCTester::WebRTCTester() {
	// TODO Auto-generated constructor stub
	mRunning = false;
	mpTesterList = NULL;
}

WebRTCTester::~WebRTCTester() {
	// TODO Auto-generated destructor stub
}

bool WebRTCTester::Start(const string& stream, const string& webSocketServer, unsigned int maxCount, const string turnServer) {
	bool bFlag = true;

	LogAync(
			LOG_ERR_SYS,
			"WebRTCTester::Start( "
			"maxCount : %d "
			")",
			maxCount
			);

	mg_mgr_init(&mMgr, NULL);

	mRunning = true;
	mWebSocketServer = webSocketServer;

	char indexStr[16] = {'\0'};
	mpTesterList = new Tester[maxCount];
	for(unsigned int i = 0; i < maxCount; i++) {
		Tester *tester = &mpTesterList[i];

		sprintf(indexStr, "%u", i);
		string wholeStream = stream + indexStr;

		tester->Init(&mMgr, mWebSocketServer, wholeStream, i);
		tester->Start();
	}

	long long lastTime = getCurrentTime();
	int second = rand() % 60;

	while ( mRunning ) {
		mg_mgr_poll(&mMgr, 100);

		long long now = getCurrentTime();

		if ( now - lastTime > second * 1000 ) {
			// Update
			int clientIndex = rand() % maxCount;
			lastTime = now;
			second = rand() % 180;

			LogAync(
					LOG_WARNING,
					"WebRTCTester::Start( "
					"[Disconnect Client : %d, And Do It Next %d second] "
					")",
					clientIndex,
					second
					);

			// Disconnect Client
			Tester *tester = &mpTesterList[clientIndex];
			tester->Disconnect();
		}
	}

	mg_mgr_free(&mMgr);

	return bFlag;
}

void WebRTCTester::Stop() {
	LogAync(
			LOG_ERR_SYS,
			"WebRTCTester::Stop("
			")"
			);
	mRunning = false;
}

} /* namespace mediaserver */
