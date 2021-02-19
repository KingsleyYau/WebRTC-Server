/*
 * MainLoop.h
 *
 *  Created on: 2019/07/30
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef SERVER_MAINLOOP_H_
#define SERVER_MAINLOOP_H_

#include <signal.h>

#include <common/KSafeMap.h>
#include <common/KThread.h>

namespace mediaserver {

class MainLoopCallback {
public:
	virtual ~MainLoopCallback(){};
	virtual void OnChildExit(int pid) = 0;
};

class MainLoopObj {
public:
	MainLoopObj(int pid, MainLoopCallback *cb) {
		isExit = false;
		this->pid = pid;
		this->cb = cb;
	}

	MainLoopObj(const MainLoopObj& item) {
		pid = item.pid;
		isExit = item.isExit;
		cb = item.cb;
	}

	MainLoopObj& operator=(const MainLoopObj& item) {
		pid = item.pid;
		isExit = item.isExit;
		cb = item.cb;
		return *this;
	}

	int pid;
	bool isExit;
	MainLoopCallback *cb;
};

class MainLoopRunnable;
typedef KSafeMap<int, MainLoopObj *> MainLoopCallbackMap;
class MainLoop {
	friend class MainLoopRunnable;

public:
	static MainLoop *GetMainLoop();

public:
	MainLoop();
	virtual ~MainLoop();

	bool Start();
	void Stop(int signal = SIGTERM);
	void Exit(int signal = SIGTERM);

	void Call(int pid);
	void StartWatchChild(int pid, MainLoopCallback *cb);
	void StopWatchChild(int pid);

private:
	void MainLoopHandle();

private:
	MainLoopCallbackMap mCallbackMap;

	// 是否运行
	KMutex mRunningMutex;
	bool mRunning;

	KThread mMainLoopThread;
	MainLoopRunnable *mpMainLoopRunnable;
};

} /* namespace mediaserver */

#endif /* SERVER_MAINLOOP_H_ */
