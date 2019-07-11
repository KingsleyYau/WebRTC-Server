/*
 * IRequest.h
 *
 *  Created on: 2015-12-3
 *      Author: Max
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef REQUEST_IREQUEST_H_
#define REQUEST_IREQUEST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
using namespace std;

class IRequest {
public:
	virtual ~IRequest(){};

	/**
	 * 开始请求
	 */
	virtual bool StartRequest() = 0;

	/**
	 * 完成请求
	 * 可能超时, 或者完成
	 */
	virtual void FinisRequest(bool bSuccess) = 0;

	/**
	 * 是否需要等待返回
	 */
	virtual bool IsNeedReturn() = 0;

};

#endif /* REQUEST_IREQUEST_H_ */
