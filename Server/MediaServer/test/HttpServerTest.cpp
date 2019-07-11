/*
 * HttpServerTest.cpp
 *
 *  Created on: 2016年6月29日
 *      Author: max
 */

#include "HttpServerTest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

void recv_callback(struct ev_loop *loop, ev_io *w, int revents);
void async_recv_callback(struct ev_loop *loop, ev_async *w, int revents);

class RequestRunnable : public KRunnable {
public:
	RequestRunnable(HttpServerTest* container) {
		mContainer = container;
	}
	virtual ~RequestRunnable() {
	}
protected:
	void onRun() {
		mContainer->RequestRunnableHandle();
	}
private:
	HttpServerTest* mContainer;
};

class RespondRunnable : public KRunnable {
public:
	RespondRunnable(HttpServerTest* container) {
		mContainer = container;
	}
	virtual ~RespondRunnable() {
	}
protected:
	void onRun() {
		mContainer->RespondRunnableHandle();
	}
private:
	HttpServerTest* mContainer;
};

HttpServerTest::HttpServerTest() {
	// TODO Auto-generated constructor stub
	mIsRunning = false;

	mIp = "";
	mPort = 0;
	mTotalRequest = 0;
	mCurrentRequest = 0;

	mLoop = NULL;

	mpRequestRunnable = new RequestRunnable(this);
	mpRespondRunnable = new RespondRunnable(this);
}

HttpServerTest::~HttpServerTest() {
	// TODO Auto-generated destructor stub
	if( mpRequestRunnable ) {
		delete mpRequestRunnable;
		mpRequestRunnable = NULL;
	}

	if( mpRespondRunnable ) {
		delete mpRespondRunnable;
		mpRespondRunnable = NULL;
	}
}

void HttpServerTest::Start(
		const string& ip,
		unsigned short port,
		unsigned int iTotalRequest,
		unsigned int threadRequestCount
		) {

	printf("# HttpServerTest::Start( "
			"ip : %s, "
			"port : %u, "
			"iTotalRequest : %d, "
			"threadRequestCount : %d "
			") \n",
			ip.c_str(),
			port,
			iTotalRequest,
			threadRequestCount
			);

	mIsRunning = true;
	mIp = ip;
	mPort = port;
	mTotalRequest = iTotalRequest;
	mThreadRequestCount = threadRequestCount;

	mCurrentRequest = 0;

	mLoop = ev_loop_new(EVFLAG_AUTO);

	// 接收线程
	mRespondThread.Start(mpRespondRunnable);

	// 发送线程
//	mpRequestThread.start(mpRequestRunnable);
	mpRequestThread = new KThread*[threadRequestCount];
	for(int i = 0; i < threadRequestCount; i++) {
		mpRequestThread[i] = new KThread();
		mpRequestThread[i]->Start(mpRequestRunnable);
	}
}

bool HttpServerTest::Request(int index, char* sendBuffer, unsigned int len) {
	struct sockaddr_in mAddr;
	bzero(&mAddr, sizeof(mAddr));
	mAddr.sin_family = AF_INET;
	mAddr.sin_port = htons(mPort);
	mAddr.sin_addr.s_addr = inet_addr(mIp.c_str());

	int iFlag = 1;
	int fd;
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("# HttpServerTest::Request( Create socket error ) \n");
		return false;
	}

	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char *)&iFlag, sizeof(iFlag));
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &iFlag, sizeof(iFlag));

	if( connect(fd, (struct sockaddr *)&mAddr, sizeof(mAddr)) != -1 ) {
//		printf("# HttpServerTest::Request( Connect ok, index : %d ) \n", index);
		if( send(fd, sendBuffer, strlen(sendBuffer), 0) > 0 ) {
			// 开始监听接收
			mRequestList.PushBack(fd);
			ev_async_send(mLoop, &mAsync_send_watcher);
			return true;
		} else {
			// 关闭连接
			close(fd);
		}
	} else {
		printf("# HttpServerTest::Request( Connect fail ) \n");
	}

	return false;
}

bool HttpServerTest::RequestPublic(int index, char* sendBuffer, unsigned int len) {
	struct sockaddr_in mAddr;
	bzero(&mAddr, sizeof(mAddr));
	mAddr.sin_family = AF_INET;
	mAddr.sin_port = htons(mPort + 1);
	mAddr.sin_addr.s_addr = inet_addr(mIp.c_str());

	int iFlag = 1;
	int fd;
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("# HttpServerTest::RequestPublic( Create socket error ) \n");
		return false;
	}

	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char *)&iFlag, sizeof(iFlag));
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &iFlag, sizeof(iFlag));

	if( connect(fd, (struct sockaddr *)&mAddr, sizeof(mAddr)) != -1 ) {
//		printf("# HttpServerTest::RequestPublic( Connect ok, index : %d ) \n", index);
		if( send(fd, sendBuffer, strlen(sendBuffer), 0) > 0 ) {
			// 开始监听接收
			mRequestList.PushBack(fd);
			ev_async_send(mLoop, &mAsync_send_watcher);
			return true;
		} else {
			// 关闭连接
			close(fd);
		}
	} else {
		printf("# HttpServerTest::RequestPublic( Connect fail ) \n");
	}

	return false;
}

void HttpServerTest::RequestRunnableHandle() {
	while( mIsRunning ) {
		mRequestKMutex.lock();
		if( mCurrentRequest < mTotalRequest ) {
			char sendBuffer[2048] = {'\0'};
			memset(sendBuffer, '\0', 2048);

			int r = mCurrentRequest % 2;
			int limit = 8;

//			time_t t;
//			long long unixTime = time(&t);

			switch(r) {
			case 0:{
				// 1.调用命令接口
//				printf("# HttpServerTest::RequestRunnableHandle( 获取跟男士有任意共同答案的问题的女士Id列表, index : %d ) \n", mCurrentRequest);
				snprintf(sendBuffer, 2048 - 1, "GET %s HTTP/1.0\r\nConection: %s\r\n\r\n",
						"/EXEC?CMD=whoami",
						"Keep-alive"
						);
				Request(mCurrentRequest, sendBuffer, strlen(sendBuffer));
				break;
			}
			case 1:{
				// 2.错误接口
//				printf("# HttpServerTest::RequestRunnableHandle( 获取有指定共同问题的女士Id列表, index : %d ) \n", mCurrentRequest);
				snprintf(sendBuffer, 2048 - 1, "GET %s HTTP/1.0\r\nConection: %s\r\n\r\n",
						"/EXE?AAA=1",
						"Keep-alive"
						);
				Request(mCurrentRequest, sendBuffer, strlen(sendBuffer));
				break;
			}
			}

			mCurrentRequest++;
			mRequestKMutex.unlock();

		} else {
			mIsRunning = false;
			mRequestKMutex.unlock();
			printf("# HttpServerTest::RequestRunnableHandle( Request %d times fnish ) \n", mCurrentRequest);
			break;
		}
	}
}

void HttpServerTest::RespondRunnableHandle() {
	// 开始主循环
	ev_io_start(mLoop, &mAccept_watcher);
	ev_set_userdata(mLoop, this);

	ev_async_init(&mAsync_send_watcher, async_recv_callback);
	ev_async_start(mLoop, &mAsync_send_watcher);

	ev_run(mLoop, 0);
}

void recv_callback(struct ev_loop *loop, ev_io *w, int revents) {
	HttpServerTest *pContainer = (HttpServerTest *)ev_userdata(loop);
	pContainer->RecvCallback(w, revents);
}

void HttpServerTest::RecvCallback(ev_io *w, int revents) {
//	printf("# HttpServerTest::RecvCallback( fd : %d ) \n", w->fd);
	// 输出返回
	char recvBuffer[2048] = {'\0'};
	memset(recvBuffer, '\0', 2048);

	if( !(revents & EV_ERROR) ) {
		int ret = recv(w->fd, recvBuffer, 2048 - 1, 0);
		if( ret > 0 ) {
//			printf("# HttpServerTest::RecvCallback( [%d bytes] :\n%s\n) \n", ret, recvBuffer);
		} else if( ret < 0 ) {
			if(errno == EAGAIN || errno == EWOULDBLOCK) {
				return;
			}
		}
	}

	// 关闭socket
	close(w->fd);
	ev_io_stop(mLoop, w);
	free(w);
}

void async_recv_callback(struct ev_loop *loop, ev_async *w, int revents) {
	HttpServerTest *pContainer = (HttpServerTest *)ev_userdata(loop);
	pContainer->AsyncRecvCallback();

}

void HttpServerTest::AsyncRecvCallback() {
	while( !mRequestList.Empty() ) {
		int fd = mRequestList.PopFront();
		ev_io *watcher = (ev_io *)malloc(sizeof(ev_io));
		ev_io_init(watcher, recv_callback, fd, EV_READ);
		ev_io_start(mLoop, watcher);
	}
}
