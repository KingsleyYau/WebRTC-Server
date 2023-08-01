/*
 * BaseRawRespond.cpp
 *
 *  Created on: 2016-3-8
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include "BaseRawRespond.h"
namespace qpidnetwork {

BaseRawRespond::BaseRawRespond() {
	// TODO Auto-generated constructor stub
	mRaw = "";
}

BaseRawRespond::~BaseRawRespond() {
	// TODO Auto-generated destructor stub
}

string BaseRawRespond::Result() const {
	return mRaw;
}

void BaseRawRespond::SetParam(const string& raw) {
	mRaw = raw;
}

} /* namespace qpidnetwork */
