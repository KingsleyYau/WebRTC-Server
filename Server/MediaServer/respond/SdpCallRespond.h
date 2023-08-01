/*
 * SdpCallRespond.h
 *
 *  Created on: 2019/07/23
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RESPOND_SDPCALLRESPOND_H_
#define RESPOND_SDPCALLRESPOND_H_

#include "BaseResultRespond.h"

// ThirdParty
#include <json/json.h>

namespace qpidnetwork {

class SdpCallRespond : public BaseResultRespond {
public:
	SdpCallRespond();
	virtual ~SdpCallRespond();

	void SetSdp(const string& sdp);
};
}
#endif /* RESPOND_SDPCALLRESPOND_H_ */
