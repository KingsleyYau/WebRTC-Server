/*
 * RPCServer.h
 *
 *  Created on: 2019/07/29
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef WEBSOCKET_RPCSERVER_H_
#define WEBSOCKET_RPCSERVER_H_

// Websocket
#include <websocket/WSServer.h>

// Request
#include <request/IRequest.h>
// Respond
#include <respond/IRespond.h>

namespace mediaserver {
class RPCServer {
public:
	RPCServer();
	virtual ~RPCServer();

public:
	bool Start(int port, int maxConnection = 100, IRequestCallback *callback = NULL);
	void Stop();

	bool SendRespond(IRequest *req, IRespond *res);
	bool SendNotice(IRequest *req);

	/***************************** WSServerCallback **************************************/
	void OnWSOpen(WSServer *server, connection_hdl hdl);
	void OnWSClose(WSServer *server, connection_hdl hdl);
	void OnWSMessage(WSServer *server, connection_hdl hdl, const string& str);
	/***************************** WSServerCallback End **************************************/

private:
	// Websocket服务
	WSServer mWSServer;
	IRequestCallback *mpCallback;
};

} /* namespace mediaserver */

#endif /* WEBSOCKET_RPCSERVER_H_ */
