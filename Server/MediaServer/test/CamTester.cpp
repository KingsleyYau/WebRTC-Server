/*
 * CamTester.cpp
 *
 *  Created on: 2019/08/21
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "CamTester.h"

#include <include/CommonHeader.h>

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
        			LOG_WARNING,
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

        }break;
        case MG_EV_WEBSOCKET_FRAME:{
            // Receive Data
        	string str((const char*)wm->data, wm->size);
        	LogAync(
        			LOG_INFO,
        			"Tester::Handle( "
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
        			LOG_WARNING,
        			"Tester::Handle( "
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

	char domain[1024] = "192.168.88.133";
	char user[1024];
	char dest[1024];
	char param[1024] = "4/SID=12346&USERTYPE=1";
	sprintf(user, "MM%d", index + 300);
	sprintf(dest, "WW%d|||PC0|||4/4/SID=12346&USERTYPE=1", index);

	this->url = url + "/" + user + "/" + domain + "/" + dest + "/" + param;
	this->stream = stream;
	this->index = index;
	this->bReconnect = bReconnect;

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
		conn = mg_connect_ws_opt(mgr, Handle, opt, url.c_str(), "", "User-Agent: Cam-Tester\r\n");
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
			FLAG_2_STRING(bFlag),
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

//	LogAync(
//			LOG_ALERT,
//			"Tester::HandleRecvData( "
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

class CamTesterRunnable : public KRunnable {
public:
	CamTesterRunnable(CamTester *container) {
		mContainer = container;
	}
	virtual ~CamTesterRunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->MainThread();
	}
private:
	CamTester *mContainer;
};

CamTester::CamTester() {
	// TODO Auto-generated constructor stub
	mpRunnable = new CamTesterRunnable(this);

	mRunning = false;
	mpTesterList = NULL;

	miReconnect = 0;
	miMaxCount = 0;
}

CamTester::~CamTester() {
	// TODO Auto-generated destructor stub
	if( mpRunnable ) {
		delete mpRunnable;
		mpRunnable = NULL;
	}
}

bool CamTester::Start(const string& stream, const string& webSocketServer, unsigned int iMaxCount, int iReconnect) {
	bool bFlag = true;

	LogAync(
			LOG_ALERT,
			"CamTester::Start( "
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
					"CamTester::Start( "
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

void CamTester::Stop() {
	LogAync(
			LOG_ALERT,
			"CamTester::Stop("
			")"
			);
	mRunning = false;
	mThread.Stop();

	mg_mgr_free(&mMgr);
}

bool CamTester::IsRunning() {
	return mRunning;
}

void CamTester::MainThread() {
	LogAync(
			LOG_NOTICE,
			"CamTester::MainThread( [Start] )"
			);

	long long lastTime = getCurrentTime();
	int second = ( miReconnect > 0 )?(rand() % miReconnect):0;
	LogAync(
			LOG_WARNING,
			"CamTester::MainThread( "
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
						LOG_WARNING,
						"CamTester::Start( "
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
			"CamTester::MainThread( [Exit] )"
			);
}

} /* namespace mediaserver */
