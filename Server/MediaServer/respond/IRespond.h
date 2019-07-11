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

class IRespond {
public:
	virtual ~IRespond(){};

	/**
	 * 获取返回数据
	 * @param buffer 数据二进制数组
	 * @param len	 数组长度
	 * @return 使用的数组长度
	 */
	virtual int GetData(char* buffer, int len, bool &more) = 0;
};

#endif /* REQUEST_IRESPOND_H_ */
