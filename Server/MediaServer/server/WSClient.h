/*
 * WSClient.h
 *
 *  Created on: 2019/07/24
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef SERVER_WSCLIENT_H_
#define SERVER_WSCLIENT_H_

#include <websocketpp/server.hpp>

using namespace websocketpp;

namespace mediaserver {

class WSClient {
	friend class WSServer;
public:
	WSClient() {
		custom = NULL;
	};
	virtual ~WSClient(){};

public:
	void *custom;

private:
	connection_hdl hdl;
};

} /* namespace mediaserver */

#endif /* SERVER_WSCLIENT_H_ */
