/*
 * SdpCallRespond.cpp
 *
 *  Created on: 2019/07/23
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "SdpCallRespond.h"

// ThirdParty
#include <json/json.h>

namespace qpidnetwork {

SdpCallRespond::SdpCallRespond() {
	// TODO Auto-generated constructor stub
}

SdpCallRespond::~SdpCallRespond() {
	// TODO Auto-generated destructor stub
}

void SdpCallRespond::SetSdp(const string& sdp) {
	SetParam(true, "");
	resRoot["sdp"] = sdp;
}

} /* namespace qpidnetwork */
