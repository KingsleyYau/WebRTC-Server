/*
 * SdpCallRequest.h
 *
 *  Created on: 2019/07/29
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef WEBSOCKET_REQUEST_SDPCALLREQUEST_H_
#define WEBSOCKET_REQUEST_SDPCALLREQUEST_H_

#include "BaseRequest.h"

namespace qpidnetwork {

class SdpCallRequest : public BaseRequest {
public:
	SdpCallRequest();
	virtual ~SdpCallRequest();

	void Parse();
};

} /* namespace qpidnetwork */

#endif /* WEBSOCKET_REQUEST_SDPCALLREQUEST_H_ */
