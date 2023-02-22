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

namespace mediaserver {

void WSEventCallback(struct mg_connection *nc, int ev, void *ev_data) {
    struct websocket_message *wm = (struct websocket_message *)ev_data;
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
        case MG_EV_WEBSOCKET_FRAME:{
            // Receive Data
//        	string str((const char*)wm->data, wm->size);
        	LogAync(
        			LOG_INFO,
        			"WSEventCallback( "
					"this:%p, "
					"[Recv_Data], "
					"%s, "
					"size:%d "
        			")",
					tester,
					tester->Desc().c_str(),
					wm->size
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
	mgr = (mg_mgr *)malloc(sizeof(mg_mgr));
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
	if(mgr) {
		free(mgr);
		mgr = NULL;
	}
}

bool CamViewerImp::Init(const string url, const string user, const string dest, int index,
		bool bReconnect, int reconnectMaxSeconds) {

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
			<< ", totalDataSize:" << totalDataSize;
	ss.unsetf(ios_base::dec);
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

		mg_mgr_init(mgr, NULL);
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
	mMutex.unlock();
}

void CamViewerImp::Poll() {
	mMutex.lock();
	if (bConnected && mgr) {
		mg_mgr_poll(mgr, 1);
	}
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
	if (bRunning && mgr) {
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

		mg_mgr_free(mgr);
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

class CamViewerRunnable:public KRunnable {
public:
	CamViewerRunnable(CamViewer *container) {
		mContainer = container;
	}
	virtual ~CamViewerRunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->MainThread();
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


CamViewer::CamViewer() {
	// TODO Auto-generated constructor stub
	mpRunnable = new CamViewerRunnable(this);
	mpReconnectRunnable = new CamViewerReconnectRunnable(this);
	mpStateRunnable = new CamViewerStateRunnable(this);

	mRunning = false;
	mpTesterList = NULL;

	miReconnect = 0;
	miMaxCount = 0;
}

CamViewer::~CamViewer() {
	// TODO Auto-generated destructor stub
	if( mpRunnable ) {
		delete mpRunnable;
		mpRunnable = NULL;
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

//	mg_mgr_init(&mMgr, NULL);

	mRunning = true;
	mWebSocketServer = webSocketServer;
	miMaxCount = iMaxCount;
	miReconnect = iReconnect;

	char indexStr[16] = {'\0'};
	mpTesterList = new CamViewerImp[iMaxCount];

	if (0 == mStateThread.Start(mpStateRunnable, "StateThread")) {
		LogAync(
				LOG_ALERT,
				"CamViewer::Start( "
				"this:%p, "
				"[Create State Thread Fail] "
				")",
				this
				);
		bFlag = false;
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

			tester->Init(wholeURL, user, room, i, (miReconnect > 0), miReconnect);
			tester->Start();
			Sleep(30);
		}
	}

	if( bFlag ) {
		if (0 == mThread.Start(mpRunnable, "MainThread")) {
			LogAync(
					LOG_ALERT,
					"CamViewer::Start( "
					"this:%p, "
					"[Create Main Thread Fail] "
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
					"[Create Reconnect Thread Fail] "
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
	mThread.Stop();
	mReconnectThread.Stop();
	mStateThread.Stop();
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

void CamViewer::MainThread() {
	LogAync(
			LOG_NOTICE,
			"CamViewer::MainThread( [Start] )"
			);

	long long lastTime = getCurrentTime();

	while ( mRunning ) {
		for(int i = 0; i < miMaxCount; i++) {
			CamViewerImp *tester = &mpTesterList[i];
			tester->Poll();
		}
		Sleep(1);
	}

	LogAync(
			LOG_NOTICE,
			"CamViewer::MainThread( [Exit] )"
			);
}

void CamViewer::ReconnectThread() {
	LogAync(
			LOG_NOTICE,
			"CamViewer::ReconnectThread( [Start] )"
			);

	while ( mRunning ) {
		for(int i = 0; i < miMaxCount; i++) {
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

	while ( mRunning ) {
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
		Sleep(10 * 1000);
	}

	LogAync(
			LOG_NOTICE,
			"CamViewer::StateThread( [Exit] )"
			);
}

} /* namespace mediaserver */
