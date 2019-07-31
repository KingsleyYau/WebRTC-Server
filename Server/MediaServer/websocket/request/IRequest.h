/*
 * IRequest.h
 *
 *  Created on: 2019/07/29
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef WEBSOCKET_REQUEST_IREQUEST_H_
#define WEBSOCKET_REQUEST_IREQUEST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
using namespace std;

// ThirdParty
#include <json/json.h>

namespace mediaserver {
class IRequest;
class IRequestCallback {
public:
	virtual ~IRequestCallback(){};
	virtual void OnRpcSdpCallRequest(IRequest *req, const string& sdp);
};

class IRequest {
public:
	IRequest();
	virtual ~IRequest();

	static IRequest *CreateRequest(const void *data, int size, bool &bParse, IRequestCallback *callback);
	virtual void Parse(const Json::Value &reqRoot, IRequestCallback *callback) = 0;

	virtual const string& GetId() = 0;
	virtual const string& GetRoute() = 0;

};

} /* namespace mediaserver */

#endif /* WEBSOCKET_REQUEST_IREQUEST_H_ */
