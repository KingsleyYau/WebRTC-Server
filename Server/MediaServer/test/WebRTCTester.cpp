/*
 * WebRTCTester.cpp
 *
 *  Created on: 2019/08/21
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "WebRTCTester.h"

#include <include/CommonHeader.h>

// ThirdParty
#include <json/json.h>
#include <common/CommonFunc.h>

namespace qpidnetwork {

void Tester::Handle(struct mg_connection *nc, int ev, void *ev_data) {
    struct websocket_message *wm = (struct websocket_message *)ev_data;
    Tester *tester = (Tester *)nc->user_data;

    switch (ev) {
		case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST:{
        	LogAync(
        			LOG_NOTICE,
        			"Tester::Handle( "
					"this : %p, "
					"[Websocket-Handshake_Request], "
					"index : %d "
        			")",
					tester,
					tester->index
        			);
		}break;
        case MG_EV_WEBSOCKET_HANDSHAKE_DONE:{
        	LogAync(
        			LOG_WARN,
        			"Tester::Handle( "
					"this : %p, "
					"[Websocket-Handshake_Done], "
					"index : %d "
        			")",
					tester,
					tester->index
        			);
        	char name[1024] = {0};
        	snprintf(name, sizeof(name), "%s", tester->stream.c_str());
        	string iceName = name;
        	tester->rtc.Start("", iceName);

        }break;
        case MG_EV_WEBSOCKET_FRAME:{
            // Receive Data
        	string str((const char*)wm->data, wm->size);
        	LogAync(
        			LOG_NOTICE,
        			"Tester::Handle( "
					"this : %p, "
					"[Websocket-Recv_Data], "
					"index : %d, "
					"size : %d,"
					"\r\n%s\r\n"
        			")",
					tester,
					tester->index,
					wm->size,
					str.c_str()
        			);
        	tester->HandleRecvData(wm->data, wm->size);

        }break;
        case MG_EV_CLOSE:{
            // Disconnect
        	LogAync(
        			LOG_WARN,
        			"Tester::Handle( "
					"this : %p, "
					"[Websocket-Close], "
					"index : %d "
        			")",
					tester,
					tester->index
        			);

        	tester->rtc.Stop();

        	if ( tester->bReconnect ) {
            	tester->Start();
        	}

        }break;
    }
}

Tester::Tester() : mMutex(KMutex::MutexType_Recursive) {
	mgr = NULL;
	conn = NULL;
	index = 0;
	bReconnect = true;
}

Tester::~Tester() {

}

bool Tester::Init(mg_mgr *mgr, const string& url, const string& stream, int index, bool bReconnect) {
	this->mgr = mgr;
	this->url = url;
	this->stream = stream;
	this->index = index;
	this->bReconnect = bReconnect;

	rtc.Init("./push.sh", "127.0.0.1", 10000 + index);
	rtc.SetCallback(this);

	return true;
}

bool Tester::Start() {
    bool bFlag = false;

	LogAync(
			LOG_NOTICE,
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
		conn = mg_connect_ws_opt(mgr, Handle, opt, url.c_str(), "", "User-Agent: WebRTCClient-Tester\r\n");
		if ( NULL != conn && conn->err == 0 ) {
			bFlag = true;
		}
		mMutex.unlock();
    }

	LogAync(
			LOG_WARN,
			"Tester::Start( "
			"this : %p, "
			"[%s], "
			"url : %s, "
			"index : %d "
			")",
			this,
			FLAG_2_STRING(bFlag),
			url.c_str(),
			index
			);

    return bFlag;
}

void Tester::Disconnect() {
	LogAync(
			LOG_WARN,
			"Tester::Disconnect( "
			"this : %p, "
			"[Websocket], "
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

void Tester::OnWebRTCClientServerSdp(WebRTCClient *rtc, const string& sdp) {
//	LogAync(
//			LOG_WARN,
//			"Tester::OnWebRTCClientServerSdp( "
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

void Tester::OnWebRTCClientStartMedia(WebRTCClient *rtc) {
	LogAync(
			LOG_WARN,
			"Tester::OnWebRTCClientStartMedia( "
			"this : %p, "
			"[WebRTCClient-开始媒体传输], "
			"rtc : %p, "
			"index : %d "
			")",
			this,
			rtc,
			index
			);
}

void Tester::OnWebRTCClientError(WebRTCClient *rtc, WebRTCClientErrorType errType, const string& errMsg) {
	LogAync(
			LOG_WARN,
			"Tester::OnWebRTCClientError( "
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

void Tester::OnWebRTCClientClose(WebRTCClient *rtc) {
	LogAync(
			LOG_WARN,
			"Tester::OnWebRTCClientClose( "
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

class WebRTCTesterRunnable : public KRunnable {
public:
	WebRTCTesterRunnable(WebRTCTester *container) {
		mContainer = container;
	}
	virtual ~WebRTCTesterRunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->MainThread();
	}
private:
	WebRTCTester *mContainer;
};

WebRTCTester::WebRTCTester() {
	// TODO Auto-generated constructor stub
	mpRunnable = new WebRTCTesterRunnable(this);

	mRunning = false;
	mpTesterList = NULL;

	miReconnect = 0;
	miMaxCount = 0;
}

WebRTCTester::~WebRTCTester() {
	// TODO Auto-generated destructor stub
	if( mpRunnable ) {
		delete mpRunnable;
		mpRunnable = NULL;
	}
}

bool WebRTCTester::Start(const string& stream, const string& webSocketServer, unsigned int iMaxCount, const string turnServer, int iReconnect) {
	bool bFlag = true;

	LogAync(
			LOG_ALERT,
			"WebRTCTester::Start( "
			"iMaxCount : %d, "
			"iReconnect : %d "
			")",
			iMaxCount,
			iReconnect
			);

	mg_mgr_init(&mMgr, NULL);

	mRunning = true;
	mWebSocketServer = webSocketServer;
	miMaxCount = iMaxCount;
	miReconnect = iReconnect;

	char indexStr[16] = {'\0'};
	mpTesterList = new Tester[iMaxCount];
	for(unsigned int i = 0; i < iMaxCount; i++) {
		Tester *tester = &mpTesterList[i];

		sprintf(indexStr, "%u", i);
		string wholeStream = stream + indexStr;

		tester->Init(&mMgr, mWebSocketServer, wholeStream, i, (miReconnect >= 0));
		tester->Start();
	}

	if( bFlag ) {
		if( 0 == mThread.Start(mpRunnable, "TestMainThread") ) {
			LogAync(
					LOG_ALERT,
					"WebRTCTester::Start( "
					"this : %p, "
					"[Create Main Thread Fail] "
					")",
					this
					);
			bFlag = false;
		}
	}

	return bFlag;
}

void WebRTCTester::Stop() {
	LogAync(
			LOG_ALERT,
			"WebRTCTester::Stop("
			")"
			);
	mRunning = false;
	mThread.Stop();

	mg_mgr_free(&mMgr);
}

bool WebRTCTester::IsRunning() {
	return mRunning;
}

void WebRTCTester::MainThread() {
	LogAync(
			LOG_NOTICE,
			"WebRTCTester::MainThread( [Start] )"
			);

	long long lastTime = getCurrentTime();
	int second = ( miReconnect > 0 )?(rand() % miReconnect):0;
	LogAync(
			LOG_WARN,
			"WebRTCTester::MainThread( "
			"[Disconnect Client After %d seconds] "
			")",
			second
			);

	while ( mRunning ) {
		mg_mgr_poll(&mMgr, 100);

		if ( miReconnect > 0 ) {
			long long now = getCurrentTime();
			if ( now - lastTime > second * 1000 ) {
				// Update
				int clientIndex = rand() % miMaxCount;
				lastTime = now;
				second = rand() % miReconnect;

				LogAync(
						LOG_WARN,
						"WebRTCTester::Start( "
						"[Disconnect Client : %d, And Do It After %d seconds] "
						")",
						clientIndex,
						second
						);

				// Disconnect Client
				Tester *tester = &mpTesterList[clientIndex];
				tester->Disconnect();
			}
		}
	}

	LogAync(
			LOG_NOTICE,
			"WebRTCTester::MainThread( [Exit] )"
			);
}

} /* namespace qpidnetwork */
