/*
 * TcpServer.cpp
 *
 *  Created on: 2016-9-12
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include "TcpServer.h"
#include "Socket.h"

#include <ev.h>

namespace qpidnetwork {
/***************************** libev回调函数 **************************************/
void accept_tcp_handler(struct ev_loop *loop, ::ev_io *w, int revents) {
	TcpServer *pTcpServer = (TcpServer *)ev_userdata(loop);
	pTcpServer->IOHandleAccept(w, revents);
}

void recv_tcp_handler(struct ev_loop *loop, ::ev_io *w, int revents) {
	TcpServer *pTcpServer = (TcpServer *)ev_userdata(loop);
	pTcpServer->IOHandleRecv(w, revents);
}
/***************************** libev回调函数 **************************************/

class IORunnable : public KRunnable {
public:
	IORunnable(TcpServer *container) {
		mContainer = container;
	}
	virtual ~IORunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->IOHandleThread();
	}
private:
	TcpServer *mContainer;
};

TcpServer::TcpServer()
:mServerMutex(KMutex::MutexType_Recursive)
{
	// TODO Auto-generated constructor stub

	mpIORunnable = new IORunnable(this);
	mLoop = NULL;
	mpSocket = Socket::Create();
	mpAcceptWatcher = (::ev_io *)malloc(sizeof(::ev_io));

	mRunning = false;
	mpTcpServerCallback = NULL;
	miMaxConnection = 1;

}

TcpServer::~TcpServer() {
	// TODO Auto-generated destructor stub
	Stop();

	if (mpIORunnable) {
		delete mpIORunnable;
		mpIORunnable = NULL;
	}

	if (mpSocket) {
		Socket::Destroy(mpSocket);
		mpSocket = NULL;
	}

	if (mpAcceptWatcher) {
		free(mpAcceptWatcher);
		mpAcceptWatcher = NULL;
	}
}

void TcpServer::SetTcpServerCallback(TcpServerCallback* callback) {
	mpTcpServerCallback = callback;
}

bool TcpServer::Start(int port, int maxConnection, const char *ip) {
	bool bFlag = true;

	LogAync(
			LOG_DEBUG,
			"TcpServer::Start, "
			"addr:[%s:%u], "
			"maxConnection:%d"
			,
			ip,
			port,
			maxConnection
			);

	mServerMutex.lock();
	if (mRunning) {
		Stop();
	}
	mRunning = true;
	miMaxConnection = maxConnection;

	// 创建socket
	int fd = INVALID_SOCKET;
	struct sockaddr_in ac_addr;
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		LogAync(
				LOG_ALERT,
				"TcpServer::Start, "
				"[Create socket error], "
				"addr:[%s:%u], "
				"maxConnection:%d"
				,
				ip,
				port,
				maxConnection
				);
		bFlag = false;
	}

	if (bFlag) {
		mpSocket->SetAddress(fd, ip, port);

		// 设置快速重用
		int iFlag = 1;
		setsockopt(mpSocket->fd, SOL_SOCKET, SO_REUSEADDR, &iFlag, sizeof(iFlag));
		// 设置非阻塞
		int flags = fcntl(mpSocket->fd, F_GETFL, 0);
		flags = flags | O_NONBLOCK;
		fcntl(mpSocket->fd, F_SETFL, flags);
	}

	if (bFlag) {
		// 绑定端口和IP地址
		bzero(&ac_addr, sizeof(ac_addr));
		ac_addr.sin_family = PF_INET;
		ac_addr.sin_port = htons(port);
		ac_addr.sin_addr.s_addr = INADDR_ANY;

		if ( bind(mpSocket->fd, (struct sockaddr *) &ac_addr, sizeof(struct sockaddr)) == -1) {
			LogAync(
					LOG_ALERT,
					"TcpServer::Start, "
					"[Bind socket error], "
					"addr:[%s:%u], "
					"maxConnection:%d"
					,
					ip,
					port,
					maxConnection
					);
			bFlag = false;
		}
	}

	if (bFlag) {
		if ( listen(mpSocket->fd, 1024) == -1) {
			LogAync(
					LOG_ALERT,
					"TcpServer::Start, "
					"[Listen socket error], "
					"addr:[%s:%u], "
					"maxConnection:%d"
					,
					ip,
					port,
					maxConnection
					);
			bFlag = false;
		}
	}

	if (bFlag) {
		/* create watchers */
		for(int i = 0 ; i < maxConnection; i++) {
			::ev_io *w = (::ev_io *)malloc(sizeof(::ev_io));
			if (w != NULL) {
				mWatcherList.PushBack(w);
			}
		}
		LogAync(
				LOG_DEBUG,
				"TcpServer::Start, "
				"[Create watchers OK], "
				"addr:[%s:%u], "
				"maxConnection:%d, "
				"mWatcherList:%d"
				,
				ip,
				port,
				maxConnection,
				mWatcherList.Size()
				);
	}

	if (bFlag) {
		mLoop = ev_loop_new(EVFLAG_AUTO);//EV_DEFAULT;
	}

	if (bFlag) {
		// 启动IO监听线程
		if (0 == mIOThread.Start(mpIORunnable, "TcpServer")) {
			LogAync(
					LOG_ALERT,
					"TcpServer::Start( [Create IO thread Fail], "
					"addr:[%s:%u], "
					"maxConnection:%d"
					,
					ip,
					port,
					maxConnection
					);
			bFlag = false;
		}
	}

	if (bFlag) {
		LogAync(
				LOG_DEBUG,
				"TcpServer::Start, "
				"[OK], "
				"addr:[%s:%u], "
				"maxConnection:%d"
				,
				ip,
				port,
				maxConnection
				);
	} else {
		LogAync(
				LOG_ALERT,
				"TcpServer::Start, "
				"[Fail], "
				"addr:[%s:%u], "
				"maxConnection:%d"
				,
				ip,
				port,
				maxConnection
				);
		Stop();
	}

	mServerMutex.unlock();

	return bFlag;
}

void TcpServer::Stop() {
	LogAync(
			LOG_DEBUG,
			"TcpServer::Stop, "
			"addr:[%s:%u], "
			"maxConnection:%d"
			,
			mpSocket->ip.c_str(),
			mpSocket->port,
			miMaxConnection
			);

	mServerMutex.lock();

	if (mRunning) {
		mRunning = false;

		// 停止监听socket事件
		mWatcherMutex.lock();
		ev_io_stop(mLoop, mpAcceptWatcher);
		mWatcherMutex.unlock();

		// 关掉监听socket
		mpSocket->Disconnect();

		// 等待IO线程停止
		mIOThread.Stop();

		// 关闭监听socket
		mpSocket->Close();

		// 清除监听器队列
		::ev_io* w = NULL;
		while( NULL != ( w = mWatcherList.PopFront()) ) {
			delete w;
		}

		if (mLoop) {
			ev_loop_destroy(mLoop);
		}
	}

	mServerMutex.unlock();

	LogAync(
			LOG_DEBUG,
			"TcpServer::Stop, "
			"[OK], "
			"addr:[%s:%u], "
			"maxConnection:%d"
			,
			mpSocket->ip.c_str(),
			mpSocket->port,
			miMaxConnection
			);

}

bool TcpServer::IsRunning() {
	return mRunning;
}

void TcpServer::Close() {
	// 关闭监听socket
	mpSocket->Close();
}

SocketStatus TcpServer::Read(Socket* socket, const char *data, int &len) {
	SocketStatus status = socket->Read(data, len);

	LogAync(
			LOG_DEBUG,
			"TcpServer::Read, "
			"fd:%d, "
			"socket:%p, "
			"status:%d, "
			"len:%d"
			,
			socket->fd,
			socket,
			status,
			len
			);

	if (status == SocketStatusFail) {
		// 读取数据失败, 停止监听epoll
		StopEvIO(socket->w);
	}

	return status;
}

bool TcpServer::Send(Socket* socket, const char *data, int &len) {
	return socket->Send(data, len);
}

void TcpServer::Disconnect(Socket* socket) {
	LogAync(
			LOG_DEBUG,
			"TcpServer::Disconnect, "
			"fd:%d, "
			"socket:%p, "
			"addr:[%s:%u]"
			,
			socket->fd,
			socket,
			socket->ip.c_str(),
			socket->port
			);

	// 断开连接
	socket->Disconnect();
}

void TcpServer::DisconnectSync(Socket* socket) {
	LogAync(
			LOG_DEBUG,
			"TcpServer::DisconnectSync, "
			"fd:%d, "
			"socket:%p, "
			"addr:[%s:%u]"
			,
			socket->fd,
			socket,
			socket->ip.c_str(),
			socket->port
			);

	// 马上断开, 停止监听epoll
	StopEvIO(socket->w);

	// 断开连接
	socket->Disconnect();
}

void TcpServer::Close(Socket* socket) {
	LogAync(
			LOG_DEBUG,
			"TcpServer::Close, "
			"fd:%d, "
			"socket:%p, "
			"addr:[%s:%u]"
			,
			socket->fd,
			socket,
			socket->ip.c_str(),
			socket->port
			);

	// 关掉连接socket
	socket->Close();

	// 释放内存
	Socket::Destroy(socket);
}

void TcpServer::IOHandleThread() {
	LogAync(
			LOG_DEBUG,
			"TcpServer::IOHandleThread( [Start] )"
			);

	// 把mServer放到事件监听队列
	ev_io_init(mpAcceptWatcher, accept_tcp_handler, mpSocket->fd, EV_READ);
	mpAcceptWatcher->data = mpSocket;
	ev_io_start(mLoop, mpAcceptWatcher);

	// 增加回调参数
	ev_set_userdata(mLoop, this);

	// 执行epoll_wait
	ev_run(mLoop, 0);

	LogAync(
			LOG_DEBUG,
			"TcpServer::IOHandleThread, "
			"[Exit]"
			
			);
}

void TcpServer::IOHandleAccept(::ev_io *w, int revents) {
	LogAync(
			LOG_DEBUG,
			"TcpServer::IOHandleAccept, "
			"[Start]"
			
			);

	int clientfd = 0;
	struct sockaddr_in addr;
	socklen_t iAddrLen = sizeof(struct sockaddr);
	while ( (clientfd = accept(w->fd, (struct sockaddr *)&addr, &iAddrLen)) < 0) {
		int errNo = errno;
		if ( errNo == EAGAIN || errNo == EWOULDBLOCK || errNo == EINTR) {
			LogAync(
					LOG_DEBUG,
					"TcpServer::IOHandleAccept, "
					"[EAGAIN || EWOULDBLOCK || EINTR]"
//					"fd:%d "
					
//					w->fd
					);
			continue;
		} else {
			LogAync(
					LOG_WARN,
					"TcpServer::AcceptCallback, "
					"[Accept error], "
					"errno:%d"
					,
					errNo
					);
			break;
		}
	}

	if (clientfd != INVALID_SOCKET) {
		// 创建连接结构体
		char* ip = inet_ntoa(addr.sin_addr);
		Socket* socket = Socket::Create();
		socket->SetAddress(clientfd, ip, addr.sin_port);

		bool bAccept = false;
		if (mpTcpServerCallback && (bAccept = mpTcpServerCallback->OnAccept(socket))) {
			int iFlag = 1;
			// 设置非阻塞
			int flags = fcntl(clientfd, F_GETFL, 0);
			flags = flags | O_NONBLOCK;
			fcntl(clientfd, F_SETFL, flags);

		    // 设置ACK马上回复
			setsockopt(clientfd, IPPROTO_TCP, TCP_NODELAY, &iFlag, sizeof(iFlag));
			// CloseSocketIfNeed（一般不会立即关闭而经历TIME_WAIT的过程）后想继续重用该socket
			setsockopt(clientfd, SOL_SOCKET, SO_REUSEADDR, &iFlag, sizeof(iFlag));
			// 如果在发送数据的时，希望不经历由系统缓冲区到socket缓冲区的拷贝而影响
			int nZero = 0;
			setsockopt(clientfd, SOL_SOCKET, SO_SNDBUF, &nZero, sizeof(nZero));

			/*deal with the tcp keepalive
			  iKeepAlive = 1 (check keepalive)
			  iKeepIdle = 600 (active keepalive after socket has idled for 10 minutes)
			  KeepInt = 60 (send keepalive every 1 minute after keepalive was actived)
			  iKeepCount = 3 (send keepalive 3 times before disconnect from peer)
			 */
			int iKeepAlive = 1, iKeepIdle = 60, KeepInt = 20, iKeepCount = 3;
			setsockopt(clientfd, SOL_SOCKET, SO_KEEPALIVE, (void*)&iKeepAlive, sizeof(iKeepAlive));
			setsockopt(clientfd, IPPROTO_TCP, TCP_KEEPIDLE, (void*)&iKeepIdle, sizeof(iKeepIdle));
			setsockopt(clientfd, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&KeepInt, sizeof(KeepInt));
			setsockopt(clientfd, IPPROTO_TCP, TCP_KEEPCNT, (void *)&iKeepCount, sizeof(iKeepCount));

			::ev_io *watcher = NULL;
			if ((watcher = mWatcherList.PopFront()) != NULL) {
				// 接受连接
				LogAync(
						LOG_DEBUG,
						"TcpServer::IOHandleAccept("
						"[Accept client], "
						"fd:%d, "
						"socket:%p, "
						"watcher:%p"
						,
						clientfd,
						socket,
						watcher
						);

			} else {
				watcher = (::ev_io *)malloc(sizeof(::ev_io));

				LogAync(
						LOG_WARN,
						"TcpServer::IOHandleAccept("
						"[Not enough watcher, new more], "
						"fd:%d, "
						"socket:%p, "
						"watcher:%p"
						,
						clientfd,
						socket,
						watcher
						);
			}

			watcher->data = socket;
			socket->w = watcher;
			ev_io_init(watcher, recv_tcp_handler, clientfd, EV_READ);

			mWatcherMutex.lock();
			ev_io_start(mLoop, watcher);
			mWatcherMutex.unlock();

		} else {
			LogAync(
					LOG_WARN,
					"TcpServer::IOHandleAccept, "
					"[Not allow accept client], "
					"fd:%d, "
					"socket:%p"
					" ",
					clientfd,
					socket
					);
			// 拒绝连接
			bAccept = false;
		}

		if (!bAccept) {
			// 断开连接
			Disconnect(socket);

			// 回调
			IOHandleOnDisconnect(socket);

//			// 关闭连接
//			Close(socket);
		}
	}

	LogAync(
			LOG_DEBUG,
			"TcpServer::IOHandleAccept, "
			"[Exit]"
//			"fd:%d "
			
//			clientfd
			);
}

void TcpServer::IOHandleRecv(::ev_io *w, int revents) {
	Socket* socket = (Socket *)w->data;

	LogAync(
			LOG_DEBUG,
			"TcpServer::IOHandleRecv, "
			"[Start], "
			"fd:%d, "
			"socket:%p, "
			"revents:%d"
			,
			w->fd,
			socket,
			revents
			);

	if (revents & EV_ERROR) {
		LogAync(
				LOG_DEBUG,
				"TcpServer::IOHandleRecv, "
				"[revents & EV_ERROR], "
				"fd:%d, "
				"socket:%p"
				,
				w->fd,
				socket
				);
		// 停止监听epoll
		StopEvIO(w);

		// 回调断开连接
		IOHandleOnDisconnect(socket);

	} else {
		LogAync(
				LOG_DEBUG,
				"TcpServer::IOHandleRecv, "
				"[OnRecvEvent], "
				"fd:%d, "
				"socket:%p"
				,
				w->fd,
				socket
				);
		if (mpTcpServerCallback != NULL) {
			mpTcpServerCallback->OnRecvEvent(socket);
		}
	}

	LogAync(
			LOG_DEBUG,
			"TcpServer::IOHandleRecv, "
			"[Exit], "
			"fd:%d, "
			"socket:%p"
			,
			w->fd,
			socket
			);
}

void TcpServer::IOHandleOnDisconnect(Socket* socket) {
	LogAync(
			LOG_DEBUG,
			"TcpServer::IOHandleOnDisconnect, "
			"socket:%p"
			,
			socket
			);

	// 回调断开连接
	if (mpTcpServerCallback != NULL) {
		mpTcpServerCallback->OnDisconnect(socket);
	}
}

void TcpServer::StopEvIO(::ev_io *w) {
	if (w != NULL) {
		int fd = w->fd;
		LogAync(
				LOG_DEBUG,
				"TcpServer::StopEvIO, "
				"fd:%d"
				,
				fd
				);

		// 停止监听epoll
		mWatcherMutex.lock();
		ev_io_stop(mLoop, w);
		mWatcherMutex.unlock();

		if (mWatcherList.Size() <= (size_t)miMaxConnection) {
			// 空闲的缓存小于设定值
			LogAync(
					LOG_DEBUG,
					"TcpServer::StopEvIO, "
					"[Return ev_io to idle list], "
					"fd:%d"
					,
					fd
					);

			mWatcherList.PushBack(w);
		} else {
			// 释放内存
			LogAync(
					LOG_WARN,
					"TcpServer::StopEvIO, "
					"[Delete extra ev_io], "
					"fd:%d"
					,
					fd
					);

			free(w);
		}
	}
}
}
