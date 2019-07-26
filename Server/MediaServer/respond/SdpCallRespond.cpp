/*
 * SdpCallRespond.cpp
 *
 *  Created on: 2019/07/23
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "SdpCallRespond.h"

namespace mediaserver {

SdpCallRespond::SdpCallRespond() {
	// TODO Auto-generated constructor stub
	mSdp = "";
}

SdpCallRespond::~SdpCallRespond() {
	// TODO Auto-generated destructor stub
}

int SdpCallRespond::GetData(char* buffer, int len, bool &more) {
	int ret = 0;
	more = false;

	snprintf(buffer, len, "{\"ret\":%d, \"errmsg\":\"%s\", \"sdp\":\"%s\"}", mRet, mErrMsg.c_str(), mSdp.c_str());
	ret = strlen(buffer);
	return ret;
}

void SdpCallRespond::SetSdp(const string& sdp) {
	SetParam(true, "");
	mSdp = sdp;
}

} /* namespace mediaserver */
