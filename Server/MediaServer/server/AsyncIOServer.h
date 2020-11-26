/*
 * AsyncIOServer.h
 *
 *  Created on: 2016-9-13
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef SERVER_ASYNCIOSERVER_H_
#define SERVER_ASYNCIOSERVER_H_

#include <common/KSafeList.h>

#include "TcpServer.h"
#include "Client.h"

namespace mediaserver {
class Client;
class AsyncIOServerCallback {
public:
	virtual ~AsyncIOServerCallback(){};
	virtual bool OnAccept(Client *client) = 0;
	virtual void OnDisconnect(Client* client) = 0;
};

class RecvRunnable;
class AsyncIOServer : public TcpServerCallback {
public:
	AsyncIOServer();
	virtual ~AsyncIOServer();

	void SetAsyncIOServerCallback(AsyncIOServerCallback* callback);
	bool Start(
			unsigned port = 9201,
			int maxConnection = 1000,
			int iThreadCount = 4,
			const char *ip = "0.0.0.0"
			);
	void Stop();
	bool IsRunning();
	void Close();

	bool Send(Client* client, const char* buf, int &len);
	void Disconnect(Client* client);

	// 获取当前处理队列大小
	unsigned int GetHandleCount();

public:
	// TcpServerCallback Implement
	bool OnAccept(Socket* socket);
	void OnRecvEvent(Socket* socket);
	void OnDisconnect(Socket* socket);

	// 处理线程
	void RecvHandleThread();

private:
	// 放到处理队列
	void PushRecvHandle(Client* client);
	// 如果已经处理完毕, 关闭连接
	bool ClientCloseIfNeed(Client* client);
	// 销毁连接
	void DestroyClient(Client* client);

	// 嵌套锁
	KMutex mServerMutex;
	bool mRunning;

	AsyncIOServerCallback* mpAsyncIOServerCallback;

	// TCP服务
	TcpServer mTcpServer;

	// 接收处理队列和线程
	ClientList mClientHandleList;

	// 空闲的Client Object
	ClientList mClientIdleList;

	// 最大连接数
	int miMaxConnection;

	// 当前现在连接数
	int miConnection;

	int mThreadCount;
	KThread** mpHandleThreads;
	RecvRunnable* mpRecvRunnable;

//	/**
//	 * 发送处理队列和线程
//	 */
//	ClientHandleList mClientHandleList;
//	KThread mSendThread;
//	SendRunnable* mpSendRunnable;
};
}
#endif /* SERVER_ASYNCIOSERVER_H_ */
