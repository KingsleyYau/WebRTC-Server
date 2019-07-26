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

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "WSClient.h"

using namespace websocketpp;

namespace mediaserver {
typedef server<config::asio> server;
typedef server::message_ptr message_ptr;

typedef KSafeMap<connection_hdl, WSClient *, std::owner_less<connection_hdl>> WSClientMap;

class WSServer;
class WSServerCallback {
public:
	virtual ~WSServerCallback(){};
	virtual void OnWSOpen(WSServer *server, connection_hdl hdl) = 0;
	virtual void OnWSClose(WSServer *server, connection_hdl hdl) = 0;
	virtual void OnWSMessage(WSServer *server, connection_hdl hdl, const string& str) = 0;
};

class WSIORunnable;
class WSServer {
	friend class WSIORunnable;

public:
	WSServer();
	virtual ~WSServer();

public:
	bool Start(int port, int maxConnection = 100);
	void Stop();

	bool SendText(connection_hdl hdl, const string& str);
	void Disconnect(connection_hdl hdl);

	void SetCallback(WSServerCallback *callback);

private:
	void IOHandleThread();
	void OnOpen(connection_hdl hdl);
	void OnClose(connection_hdl hdl);
	void OnMessage(connection_hdl hdl, message_ptr msg);

private:
	server mServer;

	KMutex mServerMutex;
	bool mRunning;

	WSIORunnable* mpIORunnable;
	KThread mIOThread;

	// 最大连接数
	int miMaxConnection;

	WSServerCallback *mpWSServerCallback;

	WSClientMap mWSClientMap;
};

} /* namespace mediaserver */

#endif /* SERVER_WSSERVER_H_ */
