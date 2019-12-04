/*
 * CmdHandler.h
 *
 *  Created on: 2019/12/04
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef CMD_CMDHANDLER_H_
#define CMD_CMDHANDLER_H_

#include <string>
using namespace std;

namespace mediaserver {

class CmdHandler {
public:
	CmdHandler();
	virtual ~CmdHandler();

public:
	bool Run(const string& cmd, const string& auth);
};

} /* namespace mediaserver */

#endif /* CMD_CMDHANDLER_H_ */
