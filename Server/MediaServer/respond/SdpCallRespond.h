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

namespace mediaserver {

class SdpCallRespond : public BaseResultRespond {
public:
	SdpCallRespond();
	virtual ~SdpCallRespond();

	int GetData(char* buffer, int len, bool &more);
	void SetSdp(const string& sdp);

private:
	string mSdp;

};
}
#endif /* RESPOND_SDPCALLRESPOND_H_ */
