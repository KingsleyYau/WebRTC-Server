/*
 * BaseRequest.cpp
 *
 *  Created on: 2015-12-3
 *      Author: Max
 *      Email: Kingsleyyau@gmail.com
 */

#include "BaseRequest.h"

namespace mediaserver {
BaseRequest::BaseRequest() {
	// TODO Auto-generated constructor stub
	mpIRequestCallback = NULL;
}

BaseRequest::~BaseRequest() {
	// TODO Auto-generated destructor stub
}

const string& BaseRequest::GetId() {
	string reqId = "";
	if( mReqRoot.isObject() ) {
		if ( mReqRoot["id"].isString() ) {
			reqId = mReqRoot["id"].asString();
		}
	}
	return reqId;
}

const string& BaseRequest::GetRoute() {
	string reqRoute = "";
	if( mReqRoot.isObject() ) {
		if ( mReqRoot["route"].isString() ) {
			reqRoute = mReqRoot["route"].asString();
		}
	}
	return reqRoute;
}

void BaseRequest::Parse(const Json::Value &reqRoot, IRequestCallback *callback) {
	mpIRequestCallback = callback;
	mReqRoot = reqRoot;
	Parse();
}

} /* namespace mediaserver */
