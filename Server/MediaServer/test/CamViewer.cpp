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

namespace mediaserver {

void CamViewerImp::Handle(struct mg_connection *nc, int ev, void *ev_data) {
    struct websocket_message *wm = (struct websocket_message *)ev_data;
    CamViewerImp *tester = (CamViewerImp *)nc->user_data;

    switch (ev) {
		case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST:{
        	LogAync(
        			LOG_INFO,
        			"CamViewerImp::Handle( "
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
        			LOG_INFO,
        			"CamViewerImp::Handle( "
					"this : %p, "
					"[Websocket-Handshake_Done], "
					"index : %d "
        			")",
					tester,
					tester->index
        			);
//        	char name[1024] = {0};
//        	snprintf(name, sizeof(name), "%s", tester->stream.c_str());
//        	string iceName = name;

        }break;
        case MG_EV_WEBSOCKET_FRAME:{
            // Receive Data
//        	string str((const char*)wm->data, wm->size);
        	LogAync(
        			LOG_INFO,
        			"CamViewerImp::Handle( "
					"this : %p, "
					"[Websocket-Recv_Data], "
					"index : %d, "
					"size : %d "
//					"\r\n%s\r\n"
        			")",
					tester,
					tester->index,
					wm->size
//					str.c_str()
        			);
        	tester->HandleRecvData(wm->data, wm->size);

        }break;
        case MG_EV_CLOSE:{
            // Disconnect
        	LogAync(
        			LOG_NOTICE,
        			"CamViewerImp::Handle( "
					"this : %p, "
					"[Websocket-Close], "
					"index : %d "
        			")",
					tester,
					tester->index
        			);

        	if ( tester->bReconnect ) {
            	tester->Start();
        	}

        }break;
    }
}

CamViewerImp::CamViewerImp() {
	mgr = NULL;
	conn = NULL;
	index = 0;
	bReconnect = true;
	mutex = NULL;
}

CamViewerImp::~CamViewerImp() {

}

bool CamViewerImp::Init(KMutex *mutex, mg_mgr *mgr, const string& url, const string& stream, int index, bool bReconnect) {
	this->mgr = mgr;

	char domain[1024] = "192.168.88.133";
	char user[1024];
	char dest[1024];
	char param[1024] = "4/SID=12346&USERTYPE=1";
	sprintf(user, "MM%d", index + 300);
	sprintf(dest, "WW%d|||PC0|||4/4/SID=12346&USERTYPE=1", index);

	this->mutex = mutex;
	this->url = url + "/" + user + "/" + domain + "/" + dest + "/" + param;
	this->stream = stream;
	this->index = index;
	this->bReconnect = bReconnect;

	return true;
}

bool CamViewerImp::Start() {
    bool bFlag = false;

	LogAync(
			LOG_INFO,
			"CamViewerImp::Start( "
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

		mutex->lock();
		conn = mg_connect_ws_opt(mgr, Handle, opt, url.c_str(), "", "User-Agent:CamViewer\r\n");
		if ( NULL != conn && conn->err == 0 ) {
			bFlag = true;
		}
		mutex->unlock();
    }

    if (!bFlag) {
		LogAync(
				LOG_WARN,
				"CamViewerImp::Start( "
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
    }
    return bFlag;
}

void CamViewerImp::Disconnect() {
	LogAync(
			LOG_NOTICE,
			"CamViewerImp::Disconnect( "
			"this : %p, "
			"[Websocket], "
			"url : %s, "
			"index : %d "
			")",
			this,
			url.c_str(),
			index
			);

	mutex->lock();
	if ( conn ) {
		mg_shutdown(conn);
		conn = NULL;
	}
	mutex->unlock();
}

bool CamViewerImp::HandleRecvData(unsigned char *data, size_t size) {
	bool bFlag = false;

//	LogAync(
//			LOG_ALERT,
//			"CamViewer::HandleRecvData( "
//			"this : %p, "
//			"url : %s, "
//			"index : %d, "
//			"size : %d "
//			")",
//			this,
//			url.c_str(),
//			index,
//			size
//			);

	return bFlag;
}

class CamViewerRunnable : public KRunnable {
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

CamViewer::CamViewer() : mMutex(KMutex::MutexType_Recursive) {
	// TODO Auto-generated constructor stub
	mpRunnable = new CamViewerRunnable(this);

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
}

bool CamViewer::Start(const string& stream, const string& webSocketServer, unsigned int iMaxCount, int iReconnect) {
	bool bFlag = true;

	LogAync(
			LOG_ALERT,
			"CamViewer::Start( "
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
	mpTesterList = new CamViewerImp[iMaxCount];
	for(unsigned int i = 0; i < iMaxCount; i++) {
		CamViewerImp *tester = &mpTesterList[i];

		sprintf(indexStr, "%u", i);
		string wholeStream = stream + indexStr;

		tester->Init(&mMutex, &mMgr, mWebSocketServer, wholeStream, i, (miReconnect >= 0));
		tester->Start();
	}

	if( bFlag ) {
		if( 0 == mThread.Start(mpRunnable, "TestMainThread") ) {
			LogAync(
					LOG_ALERT,
					"CamViewer::Start( "
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

void CamViewer::Stop() {
	LogAync(
			LOG_ALERT,
			"CamViewer::Stop("
			")"
			);
	mRunning = false;
	mThread.Stop();

	mg_mgr_free(&mMgr);
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
			"signal : %d, "
			"pid : %d "
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
	int second = (miReconnect > 0)?(rand() % miReconnect):0;
	LogAync(
			LOG_NOTICE,
			"CamViewer::MainThread( "
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
						LOG_NOTICE,
						"CamViewer::Start( "
						"[Disconnect Client : %d, And Do It After %d seconds] "
						")",
						clientIndex,
						second
						);

				// Disconnect Client
				CamViewerImp *tester = &mpTesterList[clientIndex];
				tester->Disconnect();
			}
		}
	}

	LogAync(
			LOG_NOTICE,
			"CamViewer::MainThread( [Exit] )"
			);
}

} /* namespace mediaserver */
