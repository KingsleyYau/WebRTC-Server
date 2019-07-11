/*
 * BaseRespond.h
 *
 *  Created on: 2016-3-11
 *      Author: max
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef RESPOND_BASERESPOND_H_
#define RESPOND_BASERESPOND_H_

#include "IRespond.h"
class BaseRespond : public IRespond {
public:
	BaseRespond();
	virtual ~BaseRespond();

	int GetData(char* buffer, int len, bool &more);
};

#endif /* RESPOND_BASERESPOND_H_ */
