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
	/**
	 * 命令请求线程处理
	 */
	void CmdHandle();
	bool Run(CmdItem* item);

private:
	// 是否运行
	KMutex mRunningMutex;
	bool mRunning;

	// 其他
	CmdRunnable* mpCmdRunnable;
	KThread mCmdThread[8];

	// 命令请求队列
	CmdItemList mCmdItemList;
};

#endif /* CMD_CMDHANDLER_H_ */
