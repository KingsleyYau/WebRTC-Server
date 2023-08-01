/*
 * BaseResultRespond.h
 *
 *  Created on: 2016年3月8日
 *      Author: Max.Chiu
 */

#ifndef BASERESULTRESPOND_H_
#define BASERESULTRESPOND_H_

#include "BaseRespond.h"

// ThirdParty
#include <json/json.h>

namespace qpidnetwork {
class BaseResultRespond : public BaseRespond {
public:
	BaseResultRespond();
	virtual ~BaseResultRespond();

	virtual string Result() const override;

	virtual void SetParam(bool ret, string errmsg = "");

protected:
	Json::Value resRoot;
};
}
#endif /* BASERESULTRESPOND_H_ */
