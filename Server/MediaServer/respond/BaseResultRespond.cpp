/*
 * BaseResultRespond.cpp
 *
 *  Created on: 2016年3月8日
 *      Author: Max.Chiu
 */

#include "BaseResultRespond.h"

namespace qpidnetwork {

BaseResultRespond::BaseResultRespond() {
	// TODO Auto-generated constructor stub
	resRoot["ret"] = 1;
}

BaseResultRespond::~BaseResultRespond() {
	// TODO Auto-generated destructor stub
}

string BaseResultRespond::Result() const {
	Json::FastWriter writer;
	string res = writer.write(resRoot);
	return res;
}

void BaseResultRespond::SetParam(bool ret, string errmsg) {
	resRoot["ret"] = int(ret);
	resRoot["errmsg"] = errmsg;
}

} /* namespace qpidnetwork */
