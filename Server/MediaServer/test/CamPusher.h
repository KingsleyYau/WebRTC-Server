/*
 * CamPusher.h
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
class CamPusher;

class CamPusherImp : public WebRTCClientCallback {
	friend void WSEventCallback(struct mg_connection *nc, int ev, void *ev_data);

public:
	CamPusherImp();
	~CamPusherImp();

	bool Init(const string url, const string stream, int index,
			bool bReconnect = true, int reconnectMaxSeconds = 60, double pushRatio = 1.0,
			bool bTcpForce = false);
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
	void OnLogin(Json::Value resRoot, const string res = "");
	void Login(const string user);

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
	long long startTime;
	long long loginTime;
	long long loginDelta;
	int reconnectSeconds;
	int reconnectMaxSeconds;
	double pushRatio;
	bool bTcpForce;
	bool bRunning;
	bool bConnected;
	bool bLogined;
    bool bPushing;
	KMutex mMutex;
};

class CamPusherRunnable;
class CamPusherReconnectRunnable;
class CamPusherStateRunnable;
class CamPusher {
	friend class CamPusherRunnable;
	friend class CamPusherReconnectRunnable;
	friend class CamPusherStateRunnable;

public:
	CamPusher();
	virtual ~CamPusher();

	bool Start(const string& stream, const string& webSocketServer, unsigned int iMaxCount = 1, const string turnServer = "",
			int iReconnect = 60, double pushRatio = 1.0, bool bTcpForce = false);
	void Stop();
	bool IsRunning();
	void Exit(int signal);

private:
	bool Connect(CamPusher *tester);

private:
	void MainThread();
	void ReconnectThread();
	void StateThread();

private:
    mg_mgr mMgr;
    string mWebSocketServer;

    CamPusherImp *mpTesterList;

    bool mRunning;
    int miReconnect;
    int miMaxCount;

    CamPusherRunnable* mpRunnable;
	KThread mThread;

    CamPusherReconnectRunnable* mpReconnectRunnable;
	KThread mReconnectThread;

	CamPusherStateRunnable* mpStateRunnable;
	KThread mStateThread;
};

} /* namespace mediaserver */

#endif /* TEST_WEBRTCTESTER_H_ */
