/*
 * WSServer.h
 *
 *  Created on: 2019/07/23
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef SERVER_WSSERVER_H_
#define SERVER_WSSERVER_H_

#include <common/LogManager.h>
#include <common/KSafeList.h>
#include <common/KSafeMap.h>

#include <include/ForkNotice.h>

#include <websocketpp/config/asio_no_tls.hpp>
//#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>

using namespace websocketpp;

namespace mediaserver {
typedef server<config::asio> server;
//typedef server<config::asio_tls> server;
//typedef lib::shared_ptr<lib::asio::ssl::context> context_ptr;
typedef server::message_ptr message_ptr;

class WSServer;
class WSServerCallback {
public:
	virtual ~WSServerCallback(){};
	virtual void OnWSOpen(WSServer *server, connection_hdl, const string& addr, const string& userAgent) = 0;
	virtual void OnWSClose(WSServer *server, connection_hdl hdl) = 0;
	virtual void OnWSMessage(WSServer *server, connection_hdl hdl, const string& str) = 0;
};

class WSIORunnable;
class WSServer : public ForkNotice {
	friend class WSIORunnable;

public:
	WSServer();
	virtual ~WSServer();

public:
	bool Start(int port, int maxConnection = 100);
	void StopListening();
	void Stop();

	void OnForkBefore();
	void OnForkParent();
	void OnForkChild();

	bool SendText(connection_hdl hdl, const string& str);
	void Disconnect(connection_hdl hdl);
	void Close(connection_hdl hdl);

	void SetCallback(WSServerCallback *callback);

private:
	void IOHandleThread();
	bool OnValid(connection_hdl hdl);
	void OnOpen(connection_hdl hdl);
	void OnClose(connection_hdl hdl);
	void OnMessage(connection_hdl hdl, message_ptr msg);
//	context_ptr WSServer::OnTlsInit(int mode, connection_hdl hdl);

private:
	// websocketpp
	server mServer;
	// 状态
	WSServerCallback *mpWSServerCallback;
	KMutex mServerMutex;
	bool mRunning;
	// 线程
	WSIORunnable* mpIORunnable;
	KThread mIOThread;

	// 最大连接数
	int miMaxConnection;
};

} /* namespace mediaserver */

#endif /* SERVER_WSSERVER_H_ */
