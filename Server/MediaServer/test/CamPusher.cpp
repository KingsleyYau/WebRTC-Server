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
#include <common/StringHandle.h>

#include <sstream>
#include <iostream>
#include <fstream>
using namespace std;

static int gLoginCount = 0;
static int gLoginOKCount = 0;
static KMutex gLoginMutex;

namespace qpidnetwork {

void WSEventCallback(struct mg_connection *nc, int ev, void *ev_data) {
    struct websocket_message *wm = (struct websocket_message *)ev_data;
    CamPusherImp *tester = (CamPusherImp *)nc->user_data;

    switch (ev) {
		case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST:{
        	LogAync(
        			LOG_INFO,
        			"WSEventCallback( "
					"this:%p, "
					"[Handshake_Request], "
					"index:%d "
        			")",
					tester,
					tester->index
        			);
		}break;
        case MG_EV_WEBSOCKET_HANDSHAKE_DONE:{
        	LogAync(
        			LOG_INFO,
        			"WSEventCallback( "
					"this:%p, "
					"[Handshake_Done], "
					"index:%d, "
					"stream:%s "
        			")",
					tester,
					tester->index,
					tester->stream.c_str()
        			);
        	tester->Login(tester->stream);

        }break;
        case MG_EV_WEBSOCKET_FRAME:{
            // Receive Data
        	string str((const char*)wm->data, wm->size);
        	LogAync(
        			LOG_INFO,
        			"WSEventCallback( "
					"this:%p, "
					"[Recv_Data], "
					"index:%d, "
					"size:%d,"
					"\r\n%s\r\n"
        			")",
					tester,
					tester->index,
					wm->size,
					str.c_str()
        			);
        	tester->WSRecvData(wm->data, wm->size);

        }break;
        case MG_EV_CLOSE:{
            // Disconnect
        	LogAync(
        			LOG_INFO,
        			"WSEventCallback( "
					"this:%p, "
					"[Close], "
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
	mgr = (mg_mgr *)malloc(sizeof(mg_mgr));
	conn = NULL;
	index = 0;
	sid = "";
	bReconnect = true;
	bConnected = false;
	bLogined = false;
	bPushing = false;
	bRunning = false;
	bTcpForce = false;
	pushRatio = 1;
	reconnectMaxSeconds = 0;
	reconnectSeconds = 0;
	startTime = 0;
	loginTime = 0;
	loginDelta = 0;
}

CamPusherImp::~CamPusherImp() {
}

bool CamPusherImp::Init(mg_mgr *mgr, const string url, const string stream, int index,
		const string sid,
		bool bReconnect, int reconnectMaxSeconds, double pushRatio,
		bool bTcpForce) {
	this->mgr = mgr;
	this->url = url;
	this->stream = stream;
	this->index = index;
	this->sid = sid;
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
			<< ", sid:" << sid
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

    if ( url.length() > 0 ) {
		struct mg_connect_opts opt = {0};
		opt.user_data = (void *)this;

		mMutex.lock();
	    if (reconnectMaxSeconds > 0) {
	    	int r = rand() % reconnectMaxSeconds;
	    	reconnectSeconds = MAX(30, r);
	    } else {
	    	reconnectSeconds = 0;
	    }

		string user_agent = "User-Agent:CamPusher-" + to_string(index) + "\r\n";
		conn = mg_connect_ws_opt(mgr, WSEventCallback, opt, url.c_str(), "", user_agent.c_str());
		if ( NULL != conn && conn->err == 0 ) {
			bFlag = true;
			bConnected = true;
			bRunning = true;
		    startTime = getCurrentTime();
		    pingTime = getCurrentTime();
		}
		mMutex.unlock();
    }

    if (!bFlag) {
    	LogAync(
    			LOG_WARN,
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
    } else {
    	LogAync(
    			LOG_NOTICE,
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
	bConnected = false;
	conn = NULL;
	mMutex.unlock();
}

bool CamPusherImp::Timeout() {
	bool bFlag = false;
	long long now = getCurrentTime();
	if (startTime > 0 &&
			(now - startTime > reconnectSeconds * 1000)) {
		bFlag = true;
	}
	return bFlag;
}

void CamPusherImp::Disconnect() {
	mMutex.lock();
	if (bConnected && conn) {
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
	if (bRunning) {
		rtc.Stop();
	}

	mMutex.lock();
	if (bRunning) {
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

		bRunning = false;
		bPushing = false;
		bLogined = false;

		startTime = 0;
		loginTime = 0;
		loginDelta = 0;
	}
	mMutex.unlock();
}

void CamPusherImp::OnLogin(Json::Value resRoot, const string res) {
	long long now = getCurrentTime();
	loginDelta = now - loginTime;

	if (resRoot["errno"].isInt() && resRoot["errno"].asInt() == 0) {
		bLogined = true;

		gLoginMutex.lock();
		gLoginOKCount++;
		gLoginMutex.unlock();

		bool bPush = false;
		int chance = -1;
		if (pushRatio > 0) {
			chance = rand() % int(1 / pushRatio);
			if (chance == 0) {
				bPush = true;
			}
		}

		if (bPush) {
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
			rtc.Start("", stream, bTcpForce);
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
				LOG_WARN,
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
	loginTime = getCurrentTime();

	Json::Value reqRoot;
	Json::Value reqData = Json::Value::null;
	Json::FastWriter writer;

	reqRoot["id"] = "1";
	reqRoot["route"] = "imRTC/login";

	char param[2046];
	sprintf(param, "userId=%s&userType=0&sid=%s&siteId=6", user.c_str(), sid.c_str());

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
	if (bConnected && conn) {
		mg_send_websocket_frame(conn, WEBSOCKET_OP_TEXT, (const void *)req.c_str(), req.length());
	}
	mMutex.unlock();

	gLoginMutex.lock();
	gLoginCount++;
	gLoginMutex.unlock();

}

void CamPusherImp::Ping() {
	long long now = getCurrentTime();
	long long delta = now - pingTime;
	if (!bRunning || !bLogined || delta < 5 * 1000) {
		return;
	}
	pingTime = now;

	Json::Value reqRoot;
	Json::Value reqData = Json::Value::null;
	Json::FastWriter writer;

	reqRoot["id"] = "1";
	reqRoot["route"] = "imRTC/sendPing";

	string req = writer.write(reqRoot);

	LogAync(
			LOG_NOTICE,
			"CamPusherImp::Ping( "
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
	if (bConnected && conn) {
		mg_send_websocket_frame(conn, WEBSOCKET_OP_TEXT, (const void *)req.c_str(), req.length());
	}
	mMutex.unlock();
}

bool CamPusherImp::WSRecvData(unsigned char *data, size_t size) {
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
//			LOG_WARN,
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
			LOG_INFO,
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
	if (bConnected && conn) {
		mg_send_websocket_frame(conn, WEBSOCKET_OP_TEXT, (const void *)req.c_str(), req.length());
	}
	mMutex.unlock();
}

void CamPusherImp::OnWebRTCClientStartMedia(WebRTCClient *rtc) {
	LogAync(
			LOG_NOTICE,
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
	bPushing = true;
}

void CamPusherImp::OnWebRTCClientError(WebRTCClient *rtc, WebRTCClientErrorType errType, const string& errMsg) {
	LogAync(
			LOG_WARN,
			"CamPusherImp::OnWebRTCClientError( "
			"this:%p, "
			"rtc:%p, "
			"%s "
			")",
			this,
			rtc,
			Desc().c_str()
			);
	bPushing = false;
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
	bPushing = false;
	Disconnect();
}

class CamPusherPollRunnable:public KRunnable {
public:
	CamPusherPollRunnable(CamPusher *container) {
		mContainer = container;
	}
	virtual ~CamPusherPollRunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->PollThread();
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

class CamPusherStateRunnable:public KRunnable {
public:
	CamPusherStateRunnable(CamPusher *container) {
		mContainer = container;
	}
	virtual ~CamPusherStateRunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->StateThread();
	}
private:
	CamPusher *mContainer;
};


CamPusher::CamPusher():mMutex(KMutex::MutexType_Recursive) {
	// TODO Auto-generated constructor stub
	mpPollRunnable = new CamPusherPollRunnable(this);
	mpReconnectRunnable = new CamPusherReconnectRunnable(this);
	mpStateRunnable = new CamPusherStateRunnable(this);

	mRunning = false;
	mpTesterList = NULL;

	miReconnect = 0;
	miMaxCount = 0;
}

CamPusher::~CamPusher() {
	// TODO Auto-generated destructor stub
	if( mpPollRunnable ) {
		delete mpPollRunnable;
		mpPollRunnable = NULL;
	}

	if( mpReconnectRunnable ) {
		delete mpReconnectRunnable;
		mpReconnectRunnable = NULL;
	}

	if( mpStateRunnable ) {
		delete mpStateRunnable;
		mpStateRunnable = NULL;
	}
}

bool CamPusher::Start(const string& stream, const string& webSocketServer, const string& usersFileName, unsigned int iMaxCount, const string turnServer,
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

	mg_mgr_init(&mgr, NULL);

	mRunning = true;
	mWebSocketServer = webSocketServer;
	miMaxCount = iMaxCount;
	miReconnect = iReconnect;

	char indexStr[16] = {'\0'};
	mpTesterList = new CamPusherImp[iMaxCount];

	// 启动子进程监听
	if( bFlag ) {
		bFlag = MainLoop::GetMainLoop()->Start();
		if( bFlag ) {
			LogAync(
					LOG_NOTICE, "CamPusher::Start( event : [启动监听子进程循环-OK] )"
					);
		} else {
			LogAync(
					LOG_ALERT, "CamPusher::Start( event : [启动监听子进程循环-Fail] )"
					);
			printf("# CamPusher(Loop) start Fail. \n");
		}
	}

	if (bFlag) {
		ifstream ifs;
		char buf[1024];
		if (usersFileName.length() > 0 ) {
			ifs.open(usersFileName, ios::in);
		}
		for(unsigned int i = 0; i < iMaxCount; i++) {
			CamPusherImp *tester = &mpTesterList[i];
			string wholeStream = "";
			string sid = "";

			if (ifs.is_open()) {
				if(ifs.getline(buf, sizeof(buf))) {
					vector<string> parmas = StringHandle::splitWithVector(buf, ",");
					if (parmas.size() >= 2) {
						wholeStream = parmas[0];
						sid = parmas[1];
					}
				} else {
					ifs.close();
				}
			} else {
				sprintf(indexStr, "%u", i);
				wholeStream = stream + indexStr;
			}

			tester->Init(&mgr, mWebSocketServer, wholeStream, i, sid, (miReconnect > 0), miReconnect, pushRatio, bTcpForce);
			tester->Start();
		}
		ifs.close();
	}

	if( bFlag ) {
		if (0 == mPollThread.Start(mpPollRunnable, "PollThread")) {
			LogAync(
					LOG_ALERT,
					"CamPusher::Start( "
					"this:%p, "
					"[Create PollThread Fail] "
					")",
					this
					);
			bFlag = false;
		}

		if (0 == mReconnectThread.Start(mpReconnectRunnable, "ReconnectThread")) {
			LogAync(
					LOG_ALERT,
					"CamPusher::Start( "
					"this:%p, "
					"[Create ReconnectThread Fail] "
					")",
					this
					);
			bFlag = false;
		}

		if (0 == mStateThread.Start(mpStateRunnable, "StateThread")) {
			LogAync(
					LOG_ALERT,
					"CamPusher::Start( "
					"this:%p, "
					"[Create StateThread Fail] "
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
	mPollThread.Stop();
	mReconnectThread.Stop();
	mStateThread.Stop();

	for(int i = 0; i < miMaxCount; i++) {
		mMutex.lock();
		CamPusherImp *tester = &mpTesterList[i];
		tester->Close();
		mMutex.unlock();
	}

	// 停止子进程监听循环
	MainLoop::GetMainLoop()->Stop();
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

void CamPusher::PollThread() {
	LogAync(
			LOG_NOTICE,
			"CamPusher::PollThread( [Start] )"
			);

	long long lastTime = getCurrentTime();

	while ( mRunning ) {
		mMutex.lock();
		mg_mgr_poll(&mgr, 10);
		mMutex.unlock();
	}

	LogAync(
			LOG_NOTICE,
			"CamPusher::PollThread( [Exit] )"
			);
}

void CamPusher::ReconnectThread() {
	LogAync(
			LOG_NOTICE,
			"CamPusher::ReconnectThread( [Start] )"
			);

	while ( mRunning ) {
		for(int i = 0; i < miMaxCount; i++) {
			mMutex.lock();
			CamPusherImp *tester = &mpTesterList[i];
			tester->Ping();
			if (tester->bRunning && !tester->bConnected) {
				tester->Close();

//				if (tester->bReconnect) {
//					if (tester->Timeout()) {
						LogAync(
								LOG_NOTICE,
								"CamPusher::ReconnectThread( "
								"[Reconnect Client], "
								"%s "
								")",
								tester->Desc().c_str()
								);
						tester->Start();
						Sleep(30);
//					}
//				}
			} else {
				if (tester->Timeout()) {
					// Disconnect Client
					if (tester->bReconnect) {
						LogAync(
								LOG_NOTICE,
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
			mMutex.unlock();
		}
		Sleep(100);
	}

	LogAync(
			LOG_NOTICE,
			"CamPusher::ReconnectThread( [Exit] )"
			);
}

void CamPusher::StateThread() {
	LogAync(
			LOG_NOTICE,
			"CamPusher::StateThread( [Start] )"
			);

	long long lastStateTime = getCurrentTime();
	while ( mRunning ) {
		long long now = getCurrentTime();
		if (now - lastStateTime < 10 * 1000) {
			Sleep(1000);
			continue;
		}
		lastStateTime = now;

		int online = 0;
		int login = 0;
		int pushing = 0;
		double loginDelta = 0;
		int loginDeltaCount = 0;

		for(int i = 0; i < miMaxCount; i++) {
			CamPusherImp *tester = &mpTesterList[i];
			if (tester->bConnected) {
				online++;
				if (tester->bLogined) {
					login++;
					loginDelta += tester->loginDelta;
				}
				if (tester->bPushing) {
					pushing++;
				}
			}
		}

		double percent = 0;
		if (gLoginCount > 0) {
			percent = 100.0 * gLoginOKCount / gLoginCount;
		}

		double loginDeltaAvg = 0;
		if (login > 0) {
			loginDeltaAvg = 1.0 * loginDelta / login;
		}
		LogAync(
				LOG_WARN,
				"CamPusher::StateThread( "
				"[状态服务], "
				"online:%d, "
				"login:%d, "
				"pushing:%d, "
				"login times(OK/Total):%d/%d, %.2f%%, "
				"loginDeltaAvg:%.2fms "
				")",
				online,
				login,
				pushing,
				gLoginOKCount,
				gLoginCount,
				percent,
				loginDeltaAvg
				);
	}

	LogAync(
			LOG_NOTICE,
			"CamPusher::StateThread( [Exit] )"
			);
}

} /* namespace qpidnetwork */
