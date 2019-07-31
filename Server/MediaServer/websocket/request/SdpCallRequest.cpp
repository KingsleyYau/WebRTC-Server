/*
 * SdpCallRequest.cpp
 *
 *  Created on: 2019/07/29
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "SdpCallRequest.h"

namespace mediaserver {

SdpCallRequest::SdpCallRequest() {
	// TODO Auto-generated constructor stub

}

SdpCallRequest::~SdpCallRequest() {
	// TODO Auto-generated destructor stub
}

void SdpCallRequest::Parse() {
	string sdp = "";

	if ( mReqRoot["req_data"].isObject() ) {
		Json::Value reqData = mReqRoot["req_data"];
		if( reqData["sdp"].isString() ) {
			sdp = reqData["sdp"].asString();
		}
	}

	if ( mpIRequestCallback ) {
		mpIRequestCallback->OnRpcSdpCallRequest(this, sdp);
	}
}

} /* namespace mediaserver */
