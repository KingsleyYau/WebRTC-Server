#ifndef CMD_CMDHANDLER_H_
#define CMD_CMDHANDLER_H_

#include <common/KSafeList.h>
#include <common/KThread.h>

#include <string>
using namespace std;

class CmdItem;
typedef KSafeList<CmdItem *> CmdItemList;

class CmdRunnable;
class CmdHandler {
	friend class CmdRunnable;

public:
	static CmdHandler *GetCmdHandler();

	CmdHandler();
	virtual ~CmdHandler();

public:
	bool Start();
	void Stop();
	void Check(const string& str);

private:
	void CmdHandle();
	bool Run(CmdItem* item);

private:
	KMutex mRunningMutex;
	bool mRunning;

	CmdRunnable* mpCmdRunnable;
	KThread mCmdThread[8];

	CmdItemList mCmdItemList;
};

#endif /* CMD_CMDHANDLER_H_ */
