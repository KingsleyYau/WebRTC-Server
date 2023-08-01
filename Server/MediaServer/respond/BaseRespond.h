/*
 * BaseRespond.h
 *
 *  Created on: 2016年3月11日
 *      Author: max
 */

#ifndef REQUEST_BASERESPOND_H_
#define REQUEST_BASERESPOND_H_

#include "IRespond.h"

// STL
#include <string>
#include <map>
#include <list>
using namespace std;

namespace qpidnetwork {
class BaseRespond : public IRespond {
public:
	BaseRespond();
	virtual ~BaseRespond();

	virtual unsigned StatusCode() const override;
	virtual void SetStatusCode(unsigned statusCode) override;
	virtual string StatusMsg() const override;
	virtual string Result() const override;

private:
	unsigned statusCode;
	string statusMsg;
};
}
#endif /* REQUEST_BASERESPOND_H_ */
