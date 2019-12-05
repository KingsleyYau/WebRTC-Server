/*
 * CmdHandler.cpp
 *
 *  Created on: 2019/12/04
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "CmdHandler.h"

#include <unistd.h>

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
			pid_t pid = fork();
			if ( pid < 0 ) {
				bFlag = false;
			} else if ( pid > 0 ) {
			} else {
				for(int i = 0 ; i < getdtablesize(); i++) {
					close(i);
				}
				execlp("/bin/bash", "bash", "-c", cmd.c_str(), NULL);
//				FILE *fp = NULL;
//				if ( (fp = popen(cmd.c_str(),"r")) ) {
////					char buf[1024] = {0};
////					while(NULL != fgets(buf, sizeof(buf), fp)) {
////	//					result += buf;
////					}
//					pclose(fp);
//				}
				exit(EXIT_SUCCESS);
			}
//			system(cmd.c_str());
			bFlag = true;
		}
	}

	return bFlag;
}

} /* namespace mediaserver */
