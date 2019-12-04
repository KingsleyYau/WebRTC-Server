/*
 * BaseRawRespond.cpp
 *
 *  Created on: 2016-3-8
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include "BaseRawRespond.h"
namespace mediaserver {
BaseRawRespond::BaseRawRespond() {
	// TODO Auto-generated constructor stub
	mRaw = "";
}

BaseRawRespond::~BaseRawRespond() {
	// TODO Auto-generated destructor stub
}

int BaseRawRespond::GetData(char* buffer, int len, bool &more) {
	int ret = 0;
	more = false;

	snprintf(buffer, len, "%s", mRaw.c_str());
	ret = strlen(buffer);
	return ret;
}

void BaseRawRespond::SetParam(const string& raw) {
	mRaw = raw;
}
}
