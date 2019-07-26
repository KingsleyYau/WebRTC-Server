/*
 * BaseResultRespond.h
 *
 *  Created on: 2016-3-8
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef RESPOND_BASERESULTRESPOND_H_
#define RESPOND_BASERESULTRESPOND_H_

#include "BaseRespond.h"

namespace mediaserver {

class BaseResultRespond : public BaseRespond {
public:
	BaseResultRespond();
	virtual ~BaseResultRespond();

	int GetData(char* buffer, int len, bool &more);
	void SetParam(bool ret, const string& errMsg);

protected:
	bool mRet;
	string mErrMsg;
};
}
#endif /* RESPOND_BASERESULTRESPOND_H_ */
