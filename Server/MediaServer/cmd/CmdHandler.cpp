/*
 * CmdHandler.cpp
 *
 *  Created on: 2019/12/04
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "CmdHandler.h"

namespace mediaserver {

CmdHandler::CmdHandler() {
	// TODO Auto-generated constructor stub

}

CmdHandler::~CmdHandler() {
	// TODO Auto-generated destructor stub
}

bool CmdHandler::Run(const string& cmd, const string& auth) {
	bool bFlag = false;

	if ( auth == "bWVkaWFzZXJ2ZXI6MTIz" ) {
		if( cmd.length() > 0 ) {
//			FILE *fp = NULL;
//			if ( (fp = popen(cmd.c_str(),"r")) ) {
//				char buf[1024] = {0};
//				while(NULL != fgets(buf, sizeof(buf), fp)) {
////					result += buf;
//				}
//				pclose(fp);
//			}
			system(cmd.c_str());
			bFlag = true;
		}
	}

	return bFlag;
}

} /* namespace mediaserver */
