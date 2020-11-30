/*
 * AsyncIOServer.cpp
 *
 *  Created on: 2016-9-13
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include "AsyncIOServer.h"

// Common
#include <common/CommonFunc.h>

namespace mediaserver {
class RecvRunnable : public KRunnable {
public:
	RecvRunnable(AsyncIOServer *container) {
		mContainer = container;
	}
	virtual ~RecvRunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->RecvHandleThread();
	}
private:
	AsyncIOServer *mContainer;
};

AsyncIOServer::AsyncIOServer()
:mServerMutex(KMutex::MutexType_Recursive)
{
	// TODO Auto-generated constructor stub
	mRunning = false;
	mpAsyncIOServerCallback = NULL;

	miMaxConnection = 1000;
	miConnection = 0;

	mThreadCount = 4;
	mpHandleThreads = NULL;
	mpRecvRunnable = new RecvRunnable(this);

	mTcpServer.SetTcpServerCallback(this);
}

AsyncIOServer::~AsyncIOServer() {
	// TODO Auto-generated destructor stub
	Stop();

	if( mpHandleThreads ) {
		delete[] mpHandleThreads;
		mpHandleThreads = NULL;
	}

	if( mpRecvRunnable ) {
		delete mpRecvRunnable;
		mpRecvRunnable = NULL;
	}
}

void AsyncIOServer::SetAsyncIOServerCallback(AsyncIOServerCallback* callback) {
	mpAsyncIOServerCallback = callback;
}

bool AsyncIOServer::Start(
		unsigned port,
		int maxConnection,
		int iThreadCount,
		const char *ip
		) {
	bool bFlag = false;

	LogAync(
			LOG_INFO,
			"AsyncIOServer::Start( "
			"port : %u, "
			"maxConnection : %d, "
			"iThreadCount : %d, "
			"ip : %s "
			")",
			port,
			maxConnection,
			iThreadCount,
			ip
			);

	mServerMutex.lock();
	if( mRunning ) {
		Stop();
	}
	mRunning = true;
	mThreadCount = iThreadCount;
	miMaxConnection = maxConnection;
	miConnection = 0;

	// 创建内存
	for(int i = 0; i < (int)miMaxConnection; i++) {
		Client* client = Client::Create();
		mClientIdleList.PushBack(client);
	}

	// 创建处理线程
	mpHandleThreads = new KThread*[iThreadCount];
	for(int i = 0; i < iThreadCount; i++) {
		mpHandleThreads[i] = new KThread();
		mpHandleThreads[i]->Start(mpRecvRunnable, "AsyncServer");
	}

	// 开始监听socket
	bFlag = mTcpServer.Start(port, maxConnection, ip);

	if( bFlag ) {
		LogAync(
				LOG_INFO,
				"AsyncIOServer::Start( "
				"[OK], "
				"port : %d, "
				"maxConnection : %d, "
				"iThreadCount : %d, "
				"ip : %s "
				")",
				port,
				maxConnection,
				iThreadCount,
				ip
				);
	} else {
		LogAync(
				LOG_ALERT,
				"AsyncIOServer::Start( "
				"[Fail], "
				"port : %d, "
				"maxConnection : %d, "
				"iThreadCount : %d, "
				"ip : %s "
				")",
				port,
				maxConnection,
				iThreadCount,
				ip
				);
		Stop();
	}
	mServerMutex.unlock();

	return bFlag;
}

void AsyncIOServer::Stop() {
	LogAync(
			LOG_INFO,
			"AsyncIOServer::Stop( "
			")"
			);

	mServerMutex.lock();

	if( mRunning ) {
		mRunning = false;

		// 停止监听socket
		mTcpServer.Stop();

		// 停止处理线程
		if( mpHandleThreads != NULL ) {
			for(int i = 0; i < mThreadCount; i++) {
				if( mpHandleThreads[i] != NULL ) {
					mpHandleThreads[i]->Stop();
					delete mpHandleThreads[i];
					mpHandleThreads[i] = NULL;
				}
			}
		}

		// 销毁处理队列
		Client* client = NULL;
		while( NULL != ( client = mClientHandleList.PopFront() ) ) {
		}

		// 销毁空闲队列
		while( NULL != ( client = mClientIdleList.PopFront() ) ) {
			delete client;
		}

		miConnection = 0;
	}

	mServerMutex.unlock();

	LogAync(
			LOG_INFO,
			"AsyncIOServer::Stop( "
			"[OK] "
			")"
			);
}

bool AsyncIOServer::IsRunning() {
	return mRunning;
}

void AsyncIOServer::Close() {
	// 关闭监听socket
	mTcpServer.Close();
}

bool AsyncIOServer::Send(Client* client, const char* buf, int &len) {
	LogAync(
			LOG_INFO,
			"AsyncIOServer::Send( "
			"client : %p, "
			"len : %d, "
			"buf :\n%s\n"
			")",
			client,
			len,
			buf
			);
	bool bFlag = false;

	if( client ) {
		client->clientMutex.lock();
		Socket *socket = (Socket *)client->socket;
		if( !client->disconnected ) {
			bFlag = mTcpServer.Send(socket, buf, len);
			if( !bFlag ) {
				Disconnect(client);
			}
		}
		client->clientMutex.unlock();
	}

	return bFlag;
}

void AsyncIOServer::Disconnect(Client* client) {
	Socket *socket = (Socket *)client->socket;
	LogAync(
			LOG_INFO,
			"AsyncIOServer::Disconnect( "
			"client : %p, "
			"ip : %s, "
			"port : %u "
			")",
			client,
			socket->ip.c_str(),
			socket->port
			);
	if( client ) {
		client->clientMutex.lock();
		mTcpServer.Disconnect(socket);
		client->clientMutex.unlock();
	}
}

unsigned int AsyncIOServer::GetHandleCount() {
	return mClientHandleList.Size();
}

bool AsyncIOServer::OnAccept(Socket* socket) {
	bool bFlag = false;

	// 创建新连接
	Client* client = NULL;

	if( (client = mClientIdleList.PopFront()) == NULL ) {
		// 申请额外内存
		client = Client::Create();

		LogAync(
				LOG_WARNING,
				"AsyncIOServer::OnAccept( "
				"[Not enough client, new more], "
				"client : %p, "
				"miConnection : %u "
				")",
				client,
				miConnection
				);

		bFlag = true;

		// 超过一半连接数目, 释放CPU, 让处理线程处理
		Sleep(200);

	} else {
		LogAync(
				LOG_DEBUG,
				"AsyncIOServer::OnAccept( "
				"[Get client from idle list], "
				"client : %p, "
				"miConnection : %u "
				")",
				client,
				miConnection
				);
		bFlag = true;
	}

	if( bFlag ) {
		client->Reset();
		client->socket = socket;
		socket->data = client;

		LogAync(
				LOG_INFO,
				"AsyncIOServer::OnAccept( "
				"client : %p, "
				"socket : %p, "
				"ip : %s, "
				"port : %d, "
				"miConnection : %u "
				")",
				client,
				socket,
				socket->ip.c_str(),
				socket->port,
				miConnection
				);

		bFlag = false;
		if( mpAsyncIOServerCallback ) {
			bFlag = mpAsyncIOServerCallback->OnAccept(client);
		}

		mServerMutex.lock();
		// 增加在线数目
		miConnection++;
		mServerMutex.unlock();
	}

	return bFlag;
}

void AsyncIOServer::OnRecvEvent(Socket* socket) {
	Client* client = (Client *)(socket->data);
	if( client != NULL ) {
		LogAync(
				LOG_DEBUG,
				"AsyncIOServer::OnRecvEvent( "
				"[Start], "
				"client : %p, "
				"socket : %p "
				")",
				client,
				socket
				);

		// 尝试读取数据
//		char buf[READ_BUFFER_SIZE];
//		int len = sizeof(buf) - 1;
		SocketStatus status = SocketStatusFail;
		bool disconnect = false;

		client->clientMutex.lock();
		Buffer *buffer = client->GetBuffer();

		bool logicDisconnect = false;
		while (true) {
//			int len = buffer->Freespace();//READ_BUFFER_SIZE;
//			if( buffer->Freespace() <= 0 ) {
			char *buf = (char *)buffer->GetBuffer4Write();
			int len = buffer->Freespace();//READ_BUFFER_SIZE;

			if( buffer->Freespace() <= 0 ) {
//			if( true ) {
				// 没有足够的缓存空间
				LogAync(
						LOG_ERR,
						"AsyncIOServer::OnRecvEvent( "
						"[Buffer error, buffer is not enough], "
						"client : %p, "
						"socket : %p, "
						"ip : %s, "
						"port : %u "
						")",
						client,
						socket,
						socket->ip.c_str(),
						socket->port
						);
				// 同步断开连接, 并且关闭事件监听
				mTcpServer.DisconnectSync(socket);
				disconnect = true;
				break;
			}

			status = mTcpServer.Read(socket, (char *)buf, len);
			if( status == SocketStatusSuccess ) {
				// 读取数据成功, 缓存到客户端
				buffer->TossWrite(len);

				buf[len] = '\0';
				LogAync(
						LOG_INFO,
						"AsyncIOServer::OnRecvEvent( "
						"[Read OK], "
						"client : %p, "
						"socket : %p, "
						"ip : %s, "
						"port : %u, "
						"freespace : %d, "
						"len : %d, "
						"buf :\n%s\n"
						")",
						client,
						socket,
						socket->ip.c_str(),
						socket->port,
						buffer->Freespace(),
						len,
						buf
						);
				// 放到处理队列
				PushRecvHandle(client);
				// 释放CPU让处理队列执行
				break;
			} else if( status == SocketStatusTimeout ) {
				// 没有数据可读超时返回, 不处理
				LogAync(
						LOG_DEBUG,
						"AsyncIOServer::OnRecvEvent( "
						"[Nothing to read], "
						"client : %p, "
						"socket : %p "
						")",
						client,
						socket
						);
				break;
			} else {
				// 读取数据出错, 断开
				LogAync(
						LOG_DEBUG,
						"AsyncIOServer::OnRecvEvent( "
						"[Read error], "
						"client : %p, "
						"socket : %p, "
						"ip : %s, "
						"port : %u "
						")",
						client,
						socket,
						socket->ip.c_str(),
						socket->port
						);
				disconnect = true;
				break;
			}
		}

		// 如果读出错, 断开连接
		client->disconnected = disconnect;

		bool bFlag = false;
		bFlag = ClientCloseIfNeed(client);
		client->clientMutex.unlock();

		// 销毁客户端
		if( bFlag ) {
			DestroyClient(client);
		}

		LogAync(
				LOG_DEBUG,
				"AsyncIOServer::OnRecvEvent( "
				"[Exit], "
				"client : %p, "
				"socket : %p "
				")",
				client,
				socket
				);
	}
}

void AsyncIOServer::OnDisconnect(Socket* socket) {
	Client* client = (Client *)(socket->data);
	if( client != NULL ) {
		client->clientMutex.lock();
		client->disconnected = true;

		LogAync(
				LOG_DEBUG,
				"AsyncIOServer::OnDisconnect( "
				"client : %p, "
				"socket : %p, "
				"ip : %s, "
				"port : %u, "
				"recvHandleCount : %d "
				") \n",
				client,
				socket,
				socket->ip.c_str(),
				socket->port,
				client->recvHandleCount
				);

		bool bFlag = ClientCloseIfNeed(client);

		client->clientMutex.unlock();

		// 销毁客户端
		if( bFlag ) {
			DestroyClient(client);
		}
	}
}

void AsyncIOServer::RecvHandleThread() {
	LogAync(
			LOG_DEBUG, "AsyncIOServer::RecvHandleThread( [Start] )"
			);

	Client* client = NULL;
	bool bFlag = false;

	while( mRunning ) {
		if ( (client = mClientHandleList.PopFront()) ) {
			LogAync(
					LOG_DEBUG,
					"AsyncIOServer::RecvHandleThread( "
					"[Parse, Start], "
					"client : %p "
					")",
					client
					);

			client->clientMutex.lock();
			// 开始处理
			client->Parse();

			// 减少处理数
			client->recvHandleCount--;

			LogAync(
					LOG_DEBUG,
					"AsyncIOServer::RecvHandleThread( "
					"[Parse, Exit], "
					"client : %p "
					")",
					client
					);

			// 如果已经断开连接关闭socket
			bFlag = ClientCloseIfNeed(client);
			client->clientMutex.unlock();

			// 销毁客户端
			if( bFlag ) {
				DestroyClient(client);
			}

		} else {
			Sleep(100);
		}
	}

	LogAync(
			LOG_DEBUG, "AsyncIOServer::RecvHandleThread( [Exit] )"
			);
}

void AsyncIOServer::PushRecvHandle(Client* client) {
	client->clientMutex.lock();

	// 增加计数器
	client->recvHandleCount++;

	// 放到处理队列
	mClientHandleList.PushBack(client);

	client->clientMutex.unlock();
}

bool AsyncIOServer::ClientCloseIfNeed(Client* client) {
	bool bFlag = false;
	if( client->recvHandleCount == 0 && client->disconnected && !client->closed ) {
		client->closed = true;
		Socket *socket = (Socket *)client->socket;
		LogAync(
				LOG_DEBUG,
				"AsyncIOServer::ClientCloseIfNeed( "
				"client : %p, "
				"ip : %s, "
				"port : %u "
				")",
				client,
				socket->ip.c_str(),
				socket->port
				);

		if( mpAsyncIOServerCallback ) {
			mpAsyncIOServerCallback->OnDisconnect(client);
		}

		// 关闭Socket
		mTcpServer.Close(socket);

		bFlag = true;
	}

	return bFlag;
}

void AsyncIOServer::DestroyClient(Client* client) {
	mServerMutex.lock();
	// 减少在线数目
	miConnection--;
	mServerMutex.unlock();

	// 归还内存
	if( mClientIdleList.Size() <= (size_t)miMaxConnection ) {
		// 空闲的缓存小于设定值
		LogAync(
				LOG_DEBUG,
				"AsyncIOServer::DestroyClient( "
				"[Return client to idle list], "
				"client : %p "
				")",
				client
				);

		mClientIdleList.PushBack(client);

	} else {
		LogAync(
				LOG_WARNING,
				"AsyncIOServer::DestroyClient( "
				"[Delete extra client], "
				"client : %p "
				")",
				client
				);

		// 释放内存
		Client::Destroy(client);

	}
}
}
