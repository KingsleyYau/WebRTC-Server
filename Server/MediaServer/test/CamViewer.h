/*
 * CamViewer.h
 *
 *  Created on: 2019/08/21
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef TEST_CamViewer_H_
#define TEST_CamViewer_H_

#include <mongoose/mongoose.h>

// Common
#include <common/LogManager.h>
#include <common/KThread.h>

namespace mediaserver {
class CamViewer;

class CamViewerImp {
public:
	static void Handle(struct mg_connection *nc, int ev, void *ev_data);

public:
	CamViewerImp();
	~CamViewerImp();

	bool Init(KMutex *mutex, mg_mgr *mgr, const string& url, const string& stream, int index, bool bReconnect = true);
	bool Start();
	void Disconnect();

public:
	bool HandleRecvData(unsigned char *data, size_t size);

public:
	mg_mgr *mgr;
	mg_connection *conn;
	string url;
	string stream;

	int index;
	bool bReconnect;

	KMutex *mutex;
};

class CamViewerRunnable;
class CamViewer {
	friend class CamViewerRunnable;

public:
	CamViewer();
	virtual ~CamViewer();

	bool Start(const string& stream, const string& webSocketServer, unsigned int iMaxCount = 1, int iReconnect = 0);
	void Stop();
	bool IsRunning();
	void Exit(int signal);

private:
	bool Connect(CamViewer *tester);

private:
	void MainThread();

private:
    mg_mgr mMgr;
    string mWebSocketServer;

    CamViewerImp *mpTesterList;

    bool mRunning;
    int miReconnect;
    int miMaxCount;

    CamViewerRunnable* mpRunnable;
	KThread mThread;
	KMutex mMutex;
};

} /* namespace mediaserver */

#endif /* TEST_CamViewer_H_ */
