/*
 * RPCServer.cpp
 *
 *  Created on: 2019/07/29
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "RPCServer.h"

// ThirdParty
#include <json/json.h>

namespace mediaserver {

RPCServer::RPCServer() {
	// TODO Auto-generated constructor stub
	mpCallback = NULL;
}

RPCServer::~RPCServer() {
	// TODO Auto-generated destructor stub
}

bool RPCServer::Start(int port, int maxConnection, IRequestCallback *callback) {
	bool bFlag = false;

	LogAync(
			LOG_INFO,
			"RPCServer::Start( "
			"port : %u, "
			"maxConnection : %d "
			")",
			port,
			maxConnection
			);

	mpCallback = callback;
	mWSServer.Start(port, maxConnection);

	if( bFlag ) {
		LogAync(
				LOG_INFO,
				"RPCServer::Start( "
				"[OK], "
				"port : %d, "
				"maxConnection : %d "
				")",
				port,
				maxConnection
				);
	} else {
		LogAync(
				LOG_ALERT,
				"RPCServer::Start( "
				"[Fail], "
				"port : %d, "
				"maxConnection : %d "
				")",
				port,
				maxConnection
				);
		Stop();
	}

	return bFlag;
}

void RPCServer::Stop() {
	LogAync(
			LOG_INFO,
			"RPCServer::Stop( "
			")"
			);

	mWSServer.Stop();

	LogAync(
			LOG_INFO,
			"RPCServer::Stop( "
			")"
			);
}

bool RPCServer::SendRespond(IRequest *req, IRespond *res) {
	bool bFlag = false;

	LogAync(
			LOG_INFO,
			"RPCServer::SendRespond( "
			"req : %p, "
			"res : %p "
			")",
			req,
			res
			);

	if ( req ) {
		delete req;
	}

	return bFlag;
}

void RPCServer::OnWSOpen(WSServer *server, connection_hdl hdl) {
}

void RPCServer::OnWSClose(WSServer *server, connection_hdl hdl) {
}

void RPCServer::OnWSMessage(WSServer *server, connection_hdl hdl, const string& str) {
	bool bFlag = false;
	bool bParse = false;

	IRequest *req = IRequest::CreateRequest(str.c_str(), str.length(), bParse, mpCallback);
	if ( bParse ) {

	} else {
		mWSServer.Disconnect(hdl);
	}
}

} /* namespace mediaserver */
