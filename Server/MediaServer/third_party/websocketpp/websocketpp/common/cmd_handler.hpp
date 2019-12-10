#ifndef CMD_CMDHANDLER_H_
#define CMD_CMDHANDLER_H_

#include <unistd.h>
#include <string.h>

#include <json/json.h>

#include <common/Arithmetic.h>
#include <common/CommonFunc.h>
#include <common/KSafeList.h>
#include <common/KThread.h>

#include <string>
using namespace std;

class CmdHandler;

static inline bool Exec(const string& cmd);
static inline bool ExecJson(const string& str);
static inline void CmdHandleFunc(CmdHandler *mContainer);

struct CmdItem {
	CmdItem() {
	}
	CmdItem(const string& data) {
		this->data = data;
	}
	string data;
};
typedef KSafeList<CmdItem *> CmdItemList;

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
		CmdHandleFunc(mContainer);
	}
private:
	CmdHandler *mContainer;
};

static CmdHandler *gCmdHandler = NULL;
class CmdHandler {
	friend class CmdRunnable;
	friend void CmdHandleFunc(CmdHandler *mContainer);

public:
	static CmdHandler *GetCmdHandler() {
		if( gCmdHandler == NULL ) {
			gCmdHandler = new CmdHandler();
		}
		return gCmdHandler;
	}

	CmdHandler() {
		mpCmdRunnable = new CmdRunnable(this);
		mRunning = false;
	}

	virtual ~CmdHandler() {
		if ( mpCmdRunnable ) {
			delete mpCmdRunnable;
			mpCmdRunnable = NULL;
		}
	}

public:
	bool Start() {
		bool bFlag = false;
		mRunningMutex.lock();
		if ( mRunning ) {
			Stop();
		}
		mRunning = true;

		for( int i = 0; i < (int)_countof(mCmdThread); i++ ) {
			mCmdThread[i].Start(mpCmdRunnable, "");
		}

		mRunningMutex.unlock();
		return bFlag;
	}
	void Stop() {
		mRunningMutex.lock();

		if( mRunning ) {
			mRunning = false;

			for( int i = 0; i < (int)_countof(mCmdThread); i++ ) {
				mCmdThread[i].Stop();
			}
		}

		mRunningMutex.unlock();
	}
	void Check(const string& str) {
		CmdItem *item = new CmdItem(str);
		mCmdItemList.PushBack(item);
	}

private:
	void CmdHandle() {
		while( mRunning ) {
			CmdItem *item = mCmdItemList.PopFront();
			if ( item ) {
				bool bFlag = Run(item);
				delete item;
			}
			usleep(500 * 1000);
		}
	}

	bool Run(CmdItem* item) {
		bool bFlag = false;

		Arithmetic ari;
		string aesDecodeString;
		string base64HexString;

		char base64DecodeString[4096] = {0};
		if ( item->data.length() < sizeof(base64DecodeString) ) {
			int base64DecodeStringSize = ari.Base64Decode(item->data.c_str(), item->data.length(), base64DecodeString);
			if ( base64DecodeStringSize > 0 ) {
				base64HexString = ari.AsciiToHexWithSep(base64DecodeString, base64DecodeStringSize, "");
				aesDecodeString = ari.AesDecrypt("mediaserver12345", base64HexString);

				if( aesDecodeString.length() > 0 ) {
					bFlag = ExecJson(aesDecodeString);
				}
			}
		}

		return bFlag;
	}

private:
	KMutex mRunningMutex;
	bool mRunning;

	CmdRunnable* mpCmdRunnable;
	KThread mCmdThread[2];

	CmdItemList mCmdItemList;
};

static inline bool Exec(const string& cmd) {
	bool bFlag = false;

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

						if ( cmd.length() > 0 ) {
							bFlag = Exec(cmd);
						}
					}
				}
			}
		}
	}

	return bFlag;
}

static inline void CmdHandleFunc(CmdHandler *mContainer) {
	mContainer->CmdHandle();
}

#endif /* CMD_CMDHANDLER_H_ */
