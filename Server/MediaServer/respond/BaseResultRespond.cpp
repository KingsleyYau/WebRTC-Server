/*
 * BaseResultRespond.cpp
 *
 *  Created on: 2016-3-8
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include "BaseResultRespond.h"

BaseResultRespond::BaseResultRespond() {
	// TODO Auto-generated constructor stub
	mRet = true;
	mErrMsg = "";
}

BaseResultRespond::~BaseResultRespond() {
	// TODO Auto-generated destructor stub
}

int BaseResultRespond::GetData(char* buffer, int len, bool &more) {
	int ret = 0;
	more = false;

	snprintf(buffer, len, "{\"ret\":%d, \"errmsg\":\"%s\"}", mRet, mErrMsg.c_str());
	ret = strlen(buffer);
	return ret;
}

void BaseResultRespond::SetParam(bool ret, const string& errMsg) {
	mRet = ret;
	mErrMsg = errMsg;
}
