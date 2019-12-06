/*
 * CmdHandler.h
 *
 *  Created on: 2019/12/04
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef CMD_CMDHANDLER_H_
#define CMD_CMDHANDLER_H_

// Common
#include <common/KSafeList.h>
// ThirdParty
#include <json/json.h>

#include <string>
using namespace std;

namespace mediaserver {

struct CmdItem {
	CmdItem() {
	}

	CmdItem(const string& data) {
		this->data = data;
	}

	string data;
};
// CMD请求队列
typedef KSafeList<CmdItem *> CmdItemList;

class CmdHandler {
public:
	CmdHandler();
	virtual ~CmdHandler();

public:
	bool Run(CmdItem *item);
};

} /* namespace mediaserver */

#endif /* CMD_CMDHANDLER_H_ */
