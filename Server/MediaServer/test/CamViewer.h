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

namespace mediaserver {
class CamViewer;

class CamViewerImp {
	friend void WSEventCallback(struct mg_connection *nc, int ev, void *ev_data);

public:
	CamViewerImp();
	~CamViewerImp();

	bool Init(const string url, const string user, const string dest, int index,
			bool bReconnect = true, int reconnectMaxSeconds = 60);
	bool Start();
	void Disconnect();
	void Close();
	void Stop();
	void Poll();

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

class CamViewerRunnable;
class CamViewerReconnectRunnable;
class CamViewerStateRunnable;
class CamViewer {
	friend class CamViewerRunnable;
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
	void MainThread();
	void ReconnectThread();
	void StateThread();

private:
    mg_mgr mMgr;
    string mWebSocketServer;

    CamViewerImp *mpTesterList;

    bool mRunning;
    int miReconnect;
    int miMaxCount;

    CamViewerRunnable* mpRunnable;
	KThread mThread;

    CamViewerReconnectRunnable* mpReconnectRunnable;
	KThread mReconnectThread;

	CamViewerStateRunnable* mpStateRunnable;
	KThread mStateThread;
};

} /* namespace mediaserver */

#endif /* TEST_WEBRTCTESTER_H_ */
