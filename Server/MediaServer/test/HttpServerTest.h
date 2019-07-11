/*
 * HttpServerTestClient.h
 *
 *  Created on: 2016年6月29日
 *      Author: max
 */

#ifndef TEST_HTTPSERVERTEST_H_
#define TEST_HTTPSERVERTEST_H_

#include <ev.h>
#include <common/KSafeList.h>
#include <common/KThread.h>

typedef KSafeList<int> RequestList;

#include <string>
using namespace std;

class RequestRunnable;
class RespondRunnable;
class HttpServerTest {
public:
	HttpServerTest();
	virtual ~HttpServerTest();

	void Start(
			const string& ip,
			unsigned short port,
			unsigned int totalRequest,
			unsigned int threadRequestCount = 1
			);

	void RequestRunnableHandle();
	void RespondRunnableHandle();

	void AsyncRecvCallback();
	void RecvCallback(ev_io *w, int revents);

private:
	bool Request(int index, char* sendBuffer, unsigned int len);
	bool RequestPublic(int index, char* sendBuffer, unsigned int len);

	struct ev_loop *mLoop;
	ev_io mAccept_watcher;
	ev_async mAsync_send_watcher;

	RequestList mRequestList;

	/**
	 * 请求线程
	 */
	RequestRunnable* mpRequestRunnable;
	KThread** mpRequestThread;

	/**
	 * 接收线程
	 */
	RespondRunnable* mpRespondRunnable;
	KThread mRespondThread;

	/**
	 * 是否运行
	 */
	bool mIsRunning;

	string mIp;
	unsigned short mPort;
	unsigned int mTotalRequest;
	unsigned int mCurrentRequest;
	unsigned int mThreadRequestCount;
	KMutex mRequestKMutex;
};

#endif /* TEST_HTTPSERVERTEST_H_ */
