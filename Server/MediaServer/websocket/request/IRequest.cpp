/*
 * IRequest.cpp
 *
 *  Created on: 2019/07/29
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "IRequest.h"

#include "SdpCallRequest.h"

namespace mediaserver {

IRequest::IRequest() {
	// TODO Auto-generated constructor stub

}

IRequest::~IRequest() {
	// TODO Auto-generated destructor stub
}

IRequest *CreateRequest(void *data, int size, bool &bParse, IRequestCallback *callback) {
	Json::Value reqRoot;
	Json::Reader reader;
	IRequest *req = NULL;

	bParse = reader.parse((const char *)data, reqRoot, false);
	if ( bParse ) {
		if( reqRoot.isObject() ) {
			if ( reqRoot["route"].isString() ) {
				string route = reqRoot["route"].asString();
				if ( route == "imShare/sendSdpCall" ) {
					req = new SdpCallRequest();
				} else if ( route == "imShare/sendSdpUpdate" ) {

				}

				if ( req ) {
					req->Parse(reqRoot, callback);
				}
			}
		}
	}

	return req;
}

} /* namespace mediaserver */
