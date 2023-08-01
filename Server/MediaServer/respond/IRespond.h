/*
 * IRespond.h
 *
 *  Created on: 2016年3月11日
 *      Author: max
 */

#ifndef REQUEST_IRESPOND_H_
#define REQUEST_IRESPOND_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
using namespace std;
namespace qpidnetwork {
class IRespond {
public:
	virtual ~IRespond(){};

public:
	virtual unsigned StatusCode() const = 0;
	virtual void SetStatusCode(unsigned statusCode) = 0;
	virtual string StatusMsg() const = 0;
	virtual string Result() const = 0;
};
}
#endif /* REQUEST_IRESPOND_H_ */
