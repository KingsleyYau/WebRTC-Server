/*
 * BaseRawRespond.h
 *
 *  Created on: 2016-3-8
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef RESPOND_BASERAWRESPOND_H_
#define RESPOND_BASERAWRESPOND_H_

#include "BaseRespond.h"

namespace mediaserver {

class BaseRawRespond : public BaseRespond {
public:
	BaseRawRespond();
	virtual ~BaseRawRespond();

	int GetData(char* buffer, int len, bool &more);
	void SetParam(const string& raw);

protected:
	string mRaw;
};
}
#endif /* RESPOND_BASERAWRESPOND_H_ */
