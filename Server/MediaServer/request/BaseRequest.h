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

class BaseRequest : public IRequest {
public:
	BaseRequest();
	virtual ~BaseRequest();

	virtual bool IsNeedReturn();

	virtual string ToString();
};

#endif /* REQUEST_BASEREQUEST_H_ */
