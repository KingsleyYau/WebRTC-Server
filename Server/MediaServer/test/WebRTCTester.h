/*
 * WebRTCTester.h
 *
 *  Created on: 2019/08/21
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef TEST_WEBRTCTESTER_H_
#define TEST_WEBRTCTESTER_H_

#include "WebRTCClient.h"

#include <mongoose/mongoose.h>

// Common
#include <common/LogManager.h>
#include <common/KThread.h>

namespace mediaserver {
class WebRTCTester;

class Tester : public WebRTCClientCallback {
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
	void OnWebRTCClientServerSdp(WebRTCClient *rtc, const string& sdp);
	void OnWebRTCClientStartMedia(WebRTCClient *rtc);
	void OnWebRTCClientError(WebRTCClient *rtc, WebRTCClientErrorType errType, const string& errMsg);
	void OnWebRTCClientClose(WebRTCClient *rtc);

public:
	mg_mgr *mgr;
	mg_connection *conn;
	string url;
	string stream;

	WebRTCClient rtc;
	int index;
	bool bReconnect;

	KMutex mMutex;
};

class WebRTCTesterRunnable;
class WebRTCTester {
	friend class WebRTCTesterRunnable;

public:
	WebRTCTester();
	virtual ~WebRTCTester();

	bool Start(const string& stream, const string& webSocketServer, unsigned int iMaxCount = 1, const string turnServer = "", int iReconnect = 0);
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

    WebRTCTesterRunnable* mpRunnable;
	KThread mThread;
};

} /* namespace mediaserver */

#endif /* TEST_WEBRTCTESTER_H_ */
