/*
 * BaseRequest.h
 *
 *  Created on: 2015-12-3
 *      Author: Max
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef REQUEST_BASEREQUEST_H_
#define REQUEST_BASEREQUEST_H_

#include "IRequest.h"

namespace mediaserver {
class BaseRequest : public IRequest {
public:
	BaseRequest();
	virtual ~BaseRequest();

	virtual const string& GetId();
	virtual const string& GetRoute();

	void Parse(const Json::Value &reqRoot, IRequestCallback *callback);

protected:
	virtual void Parse() = 0;

protected:
	Json::Value mReqRoot;
	IRequestCallback *mpIRequestCallback;
};

} /* namespace mediaserver */

#endif /* REQUEST_BASEREQUEST_H_ */
