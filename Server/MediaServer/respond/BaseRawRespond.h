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

namespace qpidnetwork {

class BaseRawRespond : public BaseRespond {
public:
	BaseRawRespond();
	virtual ~BaseRawRespond();

	virtual string Result() const override;

	void SetParam(const string& raw);

protected:
	string mRaw;
};
}
#endif /* RESPOND_BASERAWRESPOND_H_ */
