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

namespace mediaserver {
class BaseRespond : public IRespond {
public:
	BaseRespond();
	virtual ~BaseRespond();

	virtual string Result() override;
};
}
#endif /* REQUEST_BASERESPOND_H_ */
