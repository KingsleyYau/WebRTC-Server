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
namespace mediaserver {
class IRespond {
public:
	virtual ~IRespond(){};
	virtual string Result() = 0;
};
}
#endif /* REQUEST_IRESPOND_H_ */
