/*
 * CamPusher.cpp
 *
 *  Created on: 2019/08/21
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "CamPusher.h"

#include <include/CommonHeader.h>

// ThirdParty
#include <json/json.h>
#include <common/CommonFunc.h>
#include <sstream>

namespace mediaserver {

void CamPusherImp::Handle(struct mg_connection *nc, int ev, void *ev_data) {
    struct websocket_message *wm = (struct websocket_message *)ev_data;
    CamPusherImp *tester = (CamPusherImp *)nc->user_data;

    switch (ev) {
		case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST:{
        	LogAync(
        			LOG_INFO,
        			"CamPusherImp::Handle( "
					"this:%p, "
					"[Websocket-Handshake_Request], "
					"index:%d "
        			")",
					tester,
					tester->index
        			);
		}break;
        case MG_EV_WEBSOCKET_HANDSHAKE_DONE:{
        	LogAync(
        			LOG_INFO,
        			"CamPusherImp::Handle( "
					"this:%p, "
					"[Websocket-Handshake_Done], "
					"index:%d, "
					"stream:%s "
        			")",
					tester,
					tester->index,
					tester->stream.c_str()
        			);
//        	char name[1024] = {0};
//        	snprintf(name, sizeof(name), "%s", tester->stream.c_str());
        	tester->Login(tester->stream);

        }break;
        case MG_EV_WEBSOCKET_FRAME:{
            // Receive Data
        	string str((const char*)wm->data, wm->size);
        	LogAync(
        			LOG_INFO,
        			"CamPusherImp::Handle( "
					"this:%p, "
					"[Websocket-Recv_Data], "
					"index:%d, "
					"size:%d,"
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
        			LOG_INFO,
        			"CamPusherImp::Handle( "
					"this:%p, "
					"[Websocket-Close], "
					"index:%d "
        			")",
					tester,
					tester->index
        			);
        	tester->Stop();

        }break;
    }
}

CamPusherImp::CamPusherImp():mMutex(KMutex::MutexType_Recursive) {
//	mgr = NULL;
	mgr = (mg_mgr *)malloc(sizeof(mgr));
	conn = NULL;
	index = 0;
	bReconnect = true;
}

CamPusherImp::~CamPusherImp() {
	free(mgr);
}

bool CamPusherImp::Init(mg_mgr *mgr, const string& url, const string& stream, int index,
		bool bReconnect, int reconnectMaxSeconds, double pushRatio,
		bool bTcpForce) {
//	this->mgr = mgr;
	this->url = url;
	this->stream = stream;
	this->index = index;
	this->bReconnect = bReconnect;
	this->reconnectMaxSeconds = reconnectMaxSeconds;
	this->reconnectSeconds = 0;
	this->pushRatio = pushRatio;
	this->pushRatio = MIN(1.0, this->pushRatio);
	this->pushRatio = MAX(0, this->pushRatio);
	this->bTcpForce = bTcpForce;
	rtc.Init("./push.sh", "127.0.0.1", 10000 + index);
	rtc.SetCallback(this);

	return true;
}

string CamPusherImp::Desc() {
	std::stringstream ss;
	ss << "index:" << index
			<< ", stream:" << stream
			<< ", tcp:" << string(BOOL_2_STRING(bTcpForce))
			<< ", timeout:" << reconnectSeconds;
	ss.setf(ios::fixed);
	ss.precision(2);
	ss << ", pushRatio:" << pushRatio;
	ss.unsetf(ios_base::dec);
	return ss.str();
}

bool CamPusherImp::Start() {
    bool bFlag = false;

    if (reconnectMaxSeconds > 0) {
    	reconnectSeconds = MAX(30, (rand() % reconnectMaxSeconds));
    } else {
    	reconnectSeconds = 0;
    }

	LogAync(
			LOG_NOTICE,
			"CamPusherImp::Start( "
			"this:%p, "
			"url:%s, "
			"%s "
			")",
			this,
			url.c_str(),
			Desc().c_str()
			);

    if ( url.length() > 0 ) {
		struct mg_connect_opts opt = {0};
		opt.user_data = (void *)this;

		mMutex.lock();
		mg_mgr_init(mgr, NULL);
		conn = mg_connect_ws_opt(mgr, Handle, opt, url.c_str(), "", "User-Agent:CamPusher\r\n");
		if ( NULL != conn && conn->err == 0 ) {
			bFlag = true;
			bRunning = true;
		    startTime = getCurrentTime();
		}
		mMutex.unlock();
    }

    if (!bFlag) {
    	LogAync(
    			LOG_WARNING,
    			"CamPusherImp::Start( "
    			"this:%p, "
    			"[%s], "
    			"url:%s, "
    			"%s "
    			")",
    			this,
    			FLAG_2_STRING(bFlag),
    			url.c_str(),
				Desc().c_str()
    			);
    }

    return bFlag;
}

void CamPusherImp::Stop() {
	mMutex.lock();
	bRunning = false;
	mMutex.unlock();
}

void CamPusherImp::Poll() {
	mMutex.lock();
	if (bRunning && mgr) {
		mg_mgr_poll(mgr, 100);
	}
	mMutex.unlock();
}

bool CamPusherImp::Timeout() {
	bool bFlag = false;
	long long now = getCurrentTime();
	if (bRunning &&
			startTime > 0 &&
			(now - startTime > reconnectSeconds * 1000)) {
		bFlag = true;
	}
	return bFlag;
}

void CamPusherImp::Disconnect() {
	mMutex.lock();
	if (bRunning && conn) {
		LogAync(
				LOG_NOTICE,
				"CamPusherImp::Disconnect( "
				"this:%p, "
				"url:%s, "
				"%s "
				")",
				this,
				url.c_str(),
				Desc().c_str()
				);

		mg_shutdown(conn);
		conn = NULL;
	}
	mMutex.unlock();
}

void CamPusherImp::Close() {
	LogAync(
			LOG_NOTICE,
			"CamPusherImp::Close( "
			"this:%p, "
			"url:%s, "
			"%s "
			")",
			this,
			url.c_str(),
			Desc().c_str()
			);

	mMutex.lock();
	mg_mgr_free(mgr);
	mMutex.unlock();

	rtc.Stop();
}

void CamPusherImp::OnLogin(Json::Value resRoot, const string res) {
	if (resRoot["errno"].isInt() && resRoot["errno"].asInt() == 0) {
		bool bPush = false;
		int chance = -1;
		if (pushRatio > 0) {
			chance = rand() % int(1 / pushRatio);
			if (chance == 0) {
				bPush = true;
			}
		}

		if (bPush) {
			rtc.Start("", stream, bTcpForce);
			LogAync(
					LOG_NOTICE,
					"CamPusherImp::OnLogin( "
					"this:%p, "
					"[Push], "
					"url:%s, "
					"%s "
					")",
					this,
					url.c_str(),
					Desc().c_str()
					);
		} else {
			LogAync(
					LOG_NOTICE,
					"CamPusherImp::OnLogin( "
					"this:%p, "
					"[No Push], "
					"url:%s, "
					"%s "
					")",
					this,
					url.c_str(),
					Desc().c_str()
					);
		}
	} else {
		LogAync(
				LOG_WARNING,
				"CamPusherImp::OnLogin( "
				"this:%p, "
				"[Fail], "
				"url:%s, "
				"%s, "
				"res:%s "
				")",
				this,
				url.c_str(),
				Desc().c_str(),
				res.c_str()
				);
		Disconnect();
	}
}

void CamPusherImp::Login(const string user) {
	Json::Value reqRoot;
	Json::Value reqData = Json::Value::null;
	Json::FastWriter writer;

	reqRoot["id"] = "1";
	reqRoot["route"] = "imRTC/login";

	char param[2046];
	sprintf(param, "userId=%s&userType=0&sid=t076mcgkvle5q8sgb4pf67c5ss&siteId=4", user.c_str());

	reqData["param"] = param;
	reqRoot["req_data"] = reqData;

	string req = writer.write(reqRoot);

	LogAync(
			LOG_NOTICE,
			"CamPusherImp::Login( "
			"this:%p, "
			"url:%s, "
			"%s, "
			"req:%s "
			")",
			this,
			url.c_str(),
			Desc().c_str(),
			req.c_str()
			);

	mMutex.lock();
	if (conn && bRunning) {
		mg_send_websocket_frame(conn, WEBSOCKET_OP_TEXT, (const void *)req.c_str(), req.length());
	}
	mMutex.unlock();
}

bool CamPusherImp::HandleRecvData(unsigned char *data, size_t size) {
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

			if (route == "imRTC/sendSdpAnswerNotice") {
				if ( resRoot["req_data"].isObject() ) {
					Json::Value resData = resRoot["req_data"];

					string sdp = "";
					if ( resData["sdp"].isString() ) {
						sdp = resData["sdp"].asString();
						rtc.UpdateCandidate(sdp);
					}
				}
			} else if (route == "imRTC/login") {
				OnLogin(resRoot, str);
			}
		}
	}

	return bFlag;
}

void CamPusherImp::OnWebRTCClientServerSdp(WebRTCClient *rtc, const string& sdp) {
//	LogAync(
//			LOG_WARNING,
//			"CamPusher::OnWebRTCClientServerSdp( "
//			"this:%p, "
//			"rtc:%p, "
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

	char param[2046];
	sprintf(param, "camshare?uid=%s&room=%s|||PC4|||4&site=4", stream.c_str(), stream.c_str());

	reqData["stream"] = param;
	reqRoot["req_data"] = reqData;

	string req = writer.write(reqRoot);

	LogAync(
			LOG_NOTICE,
			"CamPusherImp::OnWebRTCClientServerSdp( "
			"this:%p, "
			"url:%s, "
			"%s, "
			"sdp:%s"
			")",
			this,
			url.c_str(),
			Desc().c_str(),
			sdp.c_str()
			);

	mMutex.lock();
	if (conn && bRunning) {
		mg_send_websocket_frame(conn, WEBSOCKET_OP_TEXT, (const void *)req.c_str(), req.length());
	}
	mMutex.unlock();
}

void CamPusherImp::OnWebRTCClientStartMedia(WebRTCClient *rtc) {
	LogAync(
			LOG_WARNING,
			"CamPusherImp::OnWebRTCClientStartMedia( "
			"this:%p, "
			"[开始媒体传输], "
			"rtc:%p, "
			"%s "
			")",
			this,
			rtc,
			Desc().c_str()
			);
}

void CamPusherImp::OnWebRTCClientError(WebRTCClient *rtc, WebRTCClientErrorType errType, const string& errMsg) {
	LogAync(
			LOG_WARNING,
			"CamPusherImp::OnWebRTCClientError( "
			"this:%p, "
			"rtc:%p, "
			"%s "
			")",
			this,
			rtc,
			Desc().c_str()
			);

	Disconnect();
}

void CamPusherImp::OnWebRTCClientClose(WebRTCClient *rtc) {
	LogAync(
			LOG_NOTICE,
			"CamPusherImp::OnWebRTCClientClose( "
			"this:%p, "
			"rtc:%p, "
			"%s "
			")",
			this,
			rtc,
			Desc().c_str()
			);

	Disconnect();
}

class CamPusherRunnable:public KRunnable {
public:
	CamPusherRunnable(CamPusher *container) {
		mContainer = container;
	}
	virtual ~CamPusherRunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->MainThread();
	}
private:
	CamPusher *mContainer;
};

class CamPusherReconnectRunnable:public KRunnable {
public:
	CamPusherReconnectRunnable(CamPusher *container) {
		mContainer = container;
	}
	virtual ~CamPusherReconnectRunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->ReconnectThread();
	}
private:
	CamPusher *mContainer;
};

CamPusher::CamPusher() {
	// TODO Auto-generated constructor stub
	mpRunnable = new CamPusherRunnable(this);
	mpReconnectRunnable = new CamPusherReconnectRunnable(this);

	mRunning = false;
	mpTesterList = NULL;

	miReconnect = 0;
	miMaxCount = 0;
}

CamPusher::~CamPusher() {
	// TODO Auto-generated destructor stub
	if( mpRunnable ) {
		delete mpRunnable;
		mpRunnable = NULL;
	}

	if( mpReconnectRunnable ) {
		delete mpReconnectRunnable;
		mpReconnectRunnable = NULL;
	}
}

bool CamPusher::Start(const string& stream, const string& webSocketServer, unsigned int iMaxCount, const string turnServer,
		int iReconnect, double pushRatio, bool bTcpForce) {
	bool bFlag = true;

	LogAync(
			LOG_ALERT,
			"CamPusher::Start( "
			"############## CamPusher ############## "
			")"
			);

	LogAync(
			LOG_ALERT,
			"CamPusher::Start( "
			"Build date : %s %s "
			")",
			__DATE__,
			__TIME__
			);

	LogAync(
			LOG_ALERT,
			"CamPusher::Start( "
			"iMaxCount:%d, "
			"iReconnect:%d, "
			"pushRatio:%.2f, "
			"bTcpForce:%s "
			")",
			iMaxCount,
			iReconnect,
			pushRatio,
			BOOL_2_STRING(bTcpForce)
			);

//	mg_mgr_init(&mMgr, NULL);

	mRunning = true;
	mWebSocketServer = webSocketServer;
	miMaxCount = iMaxCount;
	miReconnect = iReconnect;

	char indexStr[16] = {'\0'};
	mpTesterList = new CamPusherImp[iMaxCount];
	for(unsigned int i = 0; i < iMaxCount; i++) {
		CamPusherImp *tester = &mpTesterList[i];

		sprintf(indexStr, "%u", i);
		string wholeStream = stream + indexStr;

		tester->Init(&mMgr, mWebSocketServer, wholeStream, i, (miReconnect >= 0), miReconnect, pushRatio, bTcpForce);
		tester->Start();
	}

	if( bFlag ) {
		if( 0 == mThread.Start(mpRunnable, "MainThread") ) {
			LogAync(
					LOG_ALERT,
					"CamPusher::Start( "
					"this:%p, "
					"[Create Main Thread Fail] "
					")",
					this
					);
			bFlag = false;
		}

		if( 0 == mReconnectThread.Start(mpReconnectRunnable, "ReconnectThread") ) {
			LogAync(
					LOG_ALERT,
					"CamPusher::Start( "
					"this:%p, "
					"[Create Reconnect Thread Fail] "
					")",
					this
					);
			bFlag = false;
		}
	}

	return bFlag;
}

void CamPusher::Stop() {
	LogAync(
			LOG_ALERT,
			"CamPusher::Stop("
			")"
			);
	mRunning = false;
	mThread.Stop();
	mReconnectThread.Stop();
//	mg_mgr_free(&mMgr);
}

bool CamPusher::IsRunning() {
	return mRunning;
}

void CamPusher::Exit(int signal) {
	pid_t pid = getpid();
	mRunning = false;

	LogAyncUnSafe(
			LOG_ALERT,
			"CamPusher::Exit( "
			"signal:%d, "
			"pid:%d "
			")",
			signal,
			pid
			);
}

void CamPusher::MainThread() {
	LogAync(
			LOG_NOTICE,
			"CamPusher::MainThread( [Start] )"
			);

	long long lastTime = getCurrentTime();

	while ( mRunning ) {
//		mg_mgr_poll(&mMgr, 100);
		for(int i = 0; i < miMaxCount; i++) {
			CamPusherImp *tester = &mpTesterList[i];
			tester->Poll();
		}
		Sleep(1);
	}

	LogAync(
			LOG_NOTICE,
			"CamPusher::MainThread( [Exit] )"
			);
}

void CamPusher::ReconnectThread() {
	LogAync(
			LOG_NOTICE,
			"CamPusher::ReconnectThread( [Start] )"
			);

	while ( mRunning ) {
		for(int i = 0; i < miMaxCount; i++) {
			CamPusherImp *tester = &mpTesterList[i];
			if (!tester->bRunning) {
				tester->Close();

				if (tester->bReconnect) {
					LogAync(
							LOG_NOTICE,
							"CamPusher::ReconnectThread( "
							"[Reconnect Client], "
							"%s "
							")",
							tester->Desc().c_str()
							);
					tester->Start();
				}
			} else {
				if (miReconnect > 0) {
					if (tester->Timeout()) {
						// Disconnect Client
						LogAync(
								LOG_INFO,
								"CamPusher::ReconnectThread( "
								"[Disconnect Client For Timeout], "
								"%s "
								")",
								tester->Desc().c_str()
								);
						tester->Disconnect();
					}
				}
			}

		}
		Sleep(1);
	}

	LogAync(
			LOG_NOTICE,
			"CamPusher::ReconnectThread( [Exit] )"
			);
}

} /* namespace mediaserver */
