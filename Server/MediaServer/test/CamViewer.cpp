/*
 * CamViewer.cpp
 *
 *  Created on: 2019/08/21
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "CamViewer.h"

#include <include/CommonHeader.h>

// ThirdParty
#include <json/json.h>
#include <common/CommonFunc.h>
#include <sstream>

static int gLoginCount = 0;
static KMutex gLoginMutex;

namespace qpidnetwork {

void WSEventCallback(struct mg_connection *nc, int ev, void *ev_data) {
    struct websocket_message *wsm = (struct websocket_message *)ev_data;
    CamViewerImp *tester = (CamViewerImp *)nc->user_data;

    switch (ev) {
		case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST:{
        	LogAync(
        			LOG_INFO,
        			"WSEventCallback( "
					"this:%p, "
					"[Handshake_Request], "
					"%s "
        			")",
					tester,
					tester->Desc().c_str()
        			);
		}break;
        case MG_EV_WEBSOCKET_HANDSHAKE_DONE:{
        	LogAync(
        			LOG_INFO,
        			"WSEventCallback( "
					"this:%p, "
					"[Handshake_Done], "
					"%s "
        			")",
					tester,
					tester->Desc().c_str()
        			);
        	tester->Login();

        }break;
        case MG_EV_WEBSOCKET_CONTROL_FRAME:{
            // Receive Data
        	LogAync(
        			LOG_INFO,
        			"WSEventCallback( "
					"this:%p, "
					"[Recv_Control], "
					"%s, "
					"size:%d, "
					"flags:%0xx "
        			")",
					tester,
					tester->Desc().c_str(),
					wsm->size,
					wsm->flags
        			);

        }break;
        case MG_EV_WEBSOCKET_FRAME:{
            // Receive Data
        	LogAync(
        			LOG_DEBUG,
        			"WSEventCallback( "
					"this:%p, "
					"[Recv_Data], "
					"%s, "
					"size:%d "
        			")",
					tester,
					tester->Desc().c_str(),
					wsm->size
        			);
        	tester->WSRecvData(wsm->data, wsm->size);

        }break;
        case MG_EV_CLOSE:{
            // Disconnect
        	LogAync(
        			LOG_INFO,
        			"WSEventCallback( "
					"this:%p, "
					"[Close], "
					"%s "
        			")",
					tester,
					tester->Desc().c_str()
        			);
        	tester->Stop();

        }break;
    }
}

CamViewerImp::CamViewerImp():mMutex(KMutex::MutexType_Recursive) {
	mgr = NULL;
	conn = NULL;
	index = 0;
	user = "";
	dest = "";
	bReconnect = true;
	bConnected = false;
	bLogined = false;
	bRunning = false;
	reconnectMaxSeconds = 0;
	reconnectSeconds = 0;
	startTime = 0;
	loginTime = 0;
	totalDataSize = 0;
}

CamViewerImp::~CamViewerImp() {

}

bool CamViewerImp::Init(mg_mgr *mgr, const string url, const string user, const string dest, int index,
		bool bReconnect, int reconnectMaxSeconds) {
	this->mgr = mgr;
	this->url = url;
	this->user = user;
	this->dest = dest;
	this->index = index;
	this->bReconnect = bReconnect;
	this->reconnectMaxSeconds = reconnectMaxSeconds;
	this->reconnectSeconds = 0;

	return true;
}

string CamViewerImp::Desc() {
	std::stringstream ss;
	ss << "index:" << index
			<< ", user:" << user
			<< ", dest:" << dest
			<< ", timeout:" << reconnectSeconds
			<< ", total:" << ReadableSize(totalDataSize);
//	ss.unsetf(ios_base::dec);
	return ss.str();
}

bool CamViewerImp::Start() {
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

		string user_agent = "User-Agent:CamViewer-" + to_string(index) + "\r\n";
		conn = mg_connect_ws_opt(mgr, WSEventCallback, opt, url.c_str(), "", user_agent.c_str());
		if ( NULL != conn && conn->err == 0 ) {
			bFlag = true;
			bConnected = true;
			bRunning = true;
		    startTime = getCurrentTime();
		}
		mMutex.unlock();
    }

    if (!bFlag) {
    	LogAync(
    			LOG_WARN,
    			"CamViewerImp::Start( "
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
    			"CamViewerImp::Start( "
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

void CamViewerImp::Stop() {
	mMutex.lock();
	bConnected = false;
	conn = NULL;
	mMutex.unlock();
}

bool CamViewerImp::Timeout() {
	bool bFlag = false;
	long long now = getCurrentTime();
	if (startTime > 0 &&
			(now - startTime > reconnectSeconds * 1000)) {
		bFlag = true;
	}
	return bFlag;
}

void CamViewerImp::Disconnect() {
	mMutex.lock();
	if (bConnected && conn) {
		LogAync(
				LOG_NOTICE,
				"CamViewerImp::Disconnect( "
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

void CamViewerImp::Close() {
	mMutex.lock();
	if (bRunning) {
		LogAync(
				LOG_NOTICE,
				"CamViewerImp::Close( "
				"this:%p, "
				"url:%s, "
				"%s "
				")",
				this,
				url.c_str(),
				Desc().c_str()
				);

		if (conn) {
			mg_close_conn(conn);
			conn = NULL;
		}

		bRunning = false;
		bLogined = false;

		startTime = 0;
		loginTime = 0;
		totalDataSize = 0;
	}
	mMutex.unlock();
}

void CamViewerImp::Login() {
	loginTime = getCurrentTime();
	bLogined = true;

	LogAync(
			LOG_NOTICE,
			"CamViewerImp::Login( "
			"this:%p, "
			"url:%s, "
			"%s "
			")",
			this,
			url.c_str(),
			Desc().c_str()
			);

	gLoginMutex.lock();
	gLoginCount++;
	gLoginMutex.unlock();
}

bool CamViewerImp::WSRecvData(unsigned char *data, size_t size) {
	bool bFlag = false;
	totalDataSize += size;
	return bFlag;
}

class CamViewerPollRunnable:public KRunnable {
public:
	CamViewerPollRunnable(CamViewer *container) {
		mContainer = container;
	}
	virtual ~CamViewerPollRunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->PollThread();
	}
private:
	CamViewer *mContainer;
};

class CamViewerReconnectRunnable:public KRunnable {
public:
	CamViewerReconnectRunnable(CamViewer *container) {
		mContainer = container;
	}
	virtual ~CamViewerReconnectRunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->ReconnectThread();
	}
private:
	CamViewer *mContainer;
};

class CamViewerStateRunnable:public KRunnable {
public:
	CamViewerStateRunnable(CamViewer *container) {
		mContainer = container;
	}
	virtual ~CamViewerStateRunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->StateThread();
	}
private:
	CamViewer *mContainer;
};


CamViewer::CamViewer():mMutex(KMutex::MutexType_Recursive) {
	// TODO Auto-generated constructor stub
	mpPollRunnable = new CamViewerPollRunnable(this);
	mpReconnectRunnable = new CamViewerReconnectRunnable(this);
	mpStateRunnable = new CamViewerStateRunnable(this);

	mRunning = false;
	mpTesterList = NULL;

	miReconnect = 0;
	miMaxCount = 0;
}

CamViewer::~CamViewer() {
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

bool CamViewer::Start(const string& stream, const string& dest, const string& webSocketServer, unsigned int iMaxCount,
		int iReconnect) {
	bool bFlag = true;

	LogAync(
			LOG_ALERT,
			"CamViewer::Start( "
			"############## CamViewer ############## "
			")"
			);

	LogAync(
			LOG_ALERT,
			"CamViewer::Start( "
			"Build date : %s %s "
			")",
			__DATE__,
			__TIME__
			);

	LogAync(
			LOG_ALERT,
			"CamViewer::Start( "
			"iMaxCount:%d, "
			"iReconnect:%d "
			")",
			iMaxCount,
			iReconnect
			);

	mg_mgr_init(&mgr, NULL);

	mRunning = true;
	mWebSocketServer = webSocketServer;
	miMaxCount = iMaxCount;
	miReconnect = iReconnect;

	char indexStr[16] = {'\0'};
	mpTesterList = new CamViewerImp[iMaxCount];

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
		for(unsigned int i = 0; i < iMaxCount; i++) {
			CamViewerImp *tester = &mpTesterList[i];

			string user = stream + to_string(i);
			string room = dest + to_string(i);
			string param = "4/SID=12346&USERTYPE=1";
			string wholeURL = "ws://" + mWebSocketServer + "/"
					+ user + "/" + "192.168.88.133" + "/"
					+ room + "|||PC0|||4/4/SID=12346&USERTYPE=1" + "/"
					+ param;

			tester->Init(&mgr, wholeURL, user, room, i, (miReconnect > 0), miReconnect);
			tester->Start();
			Sleep(30);
		}
	}

	if( bFlag ) {
		if (0 == mPollThread.Start(mpPollRunnable, "PollThread")) {
			LogAync(
					LOG_ALERT,
					"CamViewer::Start( "
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
					"CamViewer::Start( "
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
					"CamViewer::Start( "
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

void CamViewer::Stop() {
	LogAync(
			LOG_ALERT,
			"CamViewer::Stop("
			")"
			);
	mRunning = false;
	mPollThread.Stop();
	mReconnectThread.Stop();
	mStateThread.Stop();

	// 停止子进程监听循环
	MainLoop::GetMainLoop()->Stop();
}

bool CamViewer::IsRunning() {
	return mRunning;
}

void CamViewer::Exit(int signal) {
	pid_t pid = getpid();
	mRunning = false;

	LogAyncUnSafe(
			LOG_ALERT,
			"CamViewer::Exit( "
			"signal:%d, "
			"pid:%d "
			")",
			signal,
			pid
			);
}

void CamViewer::PollThread() {
	LogAync(
			LOG_NOTICE,
			"CamViewer::PollThread( [Start] )"
			);

	long long lastTime = getCurrentTime();

	while ( mRunning ) {
		mMutex.lock();
		mg_mgr_poll(&mgr, 10);
		mMutex.unlock();
	}

	LogAync(
			LOG_NOTICE,
			"CamViewer::PollThread( [Exit] )"
			);
}

void CamViewer::ReconnectThread() {
	LogAync(
			LOG_NOTICE,
			"CamViewer::ReconnectThread( [Start] )"
			);

	while ( mRunning ) {
		for(int i = 0; i < miMaxCount; i++) {
			mMutex.lock();
			CamViewerImp *tester = &mpTesterList[i];
			if (tester->bRunning && !tester->bConnected) {
				tester->Close();

//				if (tester->bReconnect) {
//					if (tester->Timeout()) {
						LogAync(
								LOG_NOTICE,
								"CamViewer::ReconnectThread( "
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
								"CamViewer::ReconnectThread( "
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
			"CamViewer::ReconnectThread( [Exit] )"
			);
}

void CamViewer::StateThread() {
	LogAync(
			LOG_NOTICE,
			"CamViewer::StateThread( [Start] )"
			);

	long long lastStateTime = getCurrentTime();
	while ( mRunning ) {
		long long now = getCurrentTime();
		if (now - lastStateTime < 10 * 1000) {
			LogAync(
					LOG_NOTICE,
					"CamViewer::StateThread( "
					"[Sleep], "
					"now: %lld, "
					"lastStateTime: %lld "
					")",
					now,
					lastStateTime
					);
			Sleep(1000);
			continue;
		}
		lastStateTime = now;

		int online = 0;
		int login = 0;

		for(int i = 0; i < miMaxCount; i++) {
			CamViewerImp *tester = &mpTesterList[i];
			if (tester->bConnected) {
				online++;
				if (tester->bLogined) {
					login++;
				}
			}
			LogAync(
					LOG_NOTICE,
					"CamViewer::StateThread( "
					"tester:%p, "
					"%s "
					")",
					tester,
					tester->Desc().c_str()
					);
		}

		double percent = 0;
		if (online > 0) {
			percent = 100.0 * login / online;
		}

		LogAync(
				LOG_WARN,
				"CamViewer::StateThread( "
				"[状态服务], "
				"online:%d, "
				"login:%d, "
				"login times:%d, %.2f%% "
				")",
				online,
				login,
				gLoginCount,
				percent
				);
	}

	LogAync(
			LOG_NOTICE,
			"CamViewer::StateThread( [Exit] )"
			);
}

} /* namespace qpidnetwork */
