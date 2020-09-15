/*
 * CamTester.h
 *
 *  Created on: 2019/08/21
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef TEST_CamTester_H_
#define TEST_CamTester_H_

#include "mongoose.h"

// Common
#include <common/LogManager.h>
#include <common/KThread.h>

namespace mediaserver {
class CamTester;

class Tester {
public:
	static void Handle(struct mg_connection *nc, int ev, void *ev_data);

public:
	Tester();
	~Tester();

	bool Init(mg_mgr *mgr, const string& url, const string& stream, int index, bool bReconnect = true);
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

	KMutex mMutex;
};

class CamTesterRunnable;
class CamTester {
	friend class CamTesterRunnable;

public:
	CamTester();
	virtual ~CamTester();

	bool Start(const string& stream, const string& webSocketServer, unsigned int iMaxCount = 1, int iReconnect = 0);
	void Stop();
	bool IsRunning();

private:
	bool Connect(Tester *tester);

private:
	void MainThread();

private:
    mg_mgr mMgr;
    string mWebSocketServer;

    Tester *mpTesterList;

    bool mRunning;
    int miReconnect;
    int miMaxCount;

    CamTesterRunnable* mpRunnable;
	KThread mThread;
};

} /* namespace mediaserver */

#endif /* TEST_CamTester_H_ */
