/*
 * CamViewer.h
 *
 *  Created on: 2019/08/21
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef TEST_WEBRTCTESTER_H_
#define TEST_WEBRTCTESTER_H_

#include "WebRTCClient.h"

#include <mongoose/mongoose.h>
#include <json/json.h>

// Common
#include <common/LogManager.h>
#include <common/KThread.h>

namespace qpidnetwork {
class CamViewer;

class CamViewerImp {
	friend void WSEventCallback(struct mg_connection *nc, int ev, void *ev_data);

public:
	CamViewerImp();
	~CamViewerImp();

	bool Init(mg_mgr *mgr, const string url, const string user, const string dest, int index,
			bool bReconnect = true, int reconnectMaxSeconds = 60);
	bool Start();
	void Disconnect();
	void Close();
	void Stop();

	string Desc();
	bool Timeout();

public:
	bool WSRecvData(unsigned char *data, size_t size);

private:
	void Login();

public:
	mg_mgr *mgr;
	mg_connection *conn;
	string url;
	string user;
	string dest;

	int index;
	bool bReconnect;
	long long startTime;
	long long loginTime;
	int reconnectSeconds;
	int reconnectMaxSeconds;
	bool bRunning;
	bool bConnected;
	bool bLogined;
	long long totalDataSize;
	KMutex mMutex;
};

class CamViewerPollRunnable;
class CamViewerReconnectRunnable;
class CamViewerStateRunnable;
class CamViewer {
	friend class CamViewerPollRunnable;
	friend class CamViewerReconnectRunnable;
	friend class CamViewerStateRunnable;

public:
	CamViewer();
	virtual ~CamViewer();

	bool Start(const string& stream, const string& dest, const string& webSocketServer, unsigned int iMaxCount = 1,
			int iReconnect = 60);
	void Stop();
	bool IsRunning();
	void Exit(int signal);

private:
	bool Connect(CamViewer *tester);

private:
	void PollThread();
	void ReconnectThread();
	void StateThread();

private:
    mg_mgr mgr;
    string mWebSocketServer;

    CamViewerImp *mpTesterList;

    bool mRunning;
    int miReconnect;
    int miMaxCount;

    CamViewerPollRunnable* mpPollRunnable;
	KThread mPollThread;

    CamViewerReconnectRunnable* mpReconnectRunnable;
	KThread mReconnectThread;

	CamViewerStateRunnable* mpStateRunnable;
	KThread mStateThread;

	KMutex mMutex;
};

} /* namespace qpidnetwork */

#endif /* TEST_WEBRTCTESTER_H_ */
