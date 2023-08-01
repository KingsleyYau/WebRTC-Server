/*
 * MainLoop.cpp
 *
 *  Created on: 2019/07/30
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "MainLoop.h"
#include <signal.h>

#include <common/LogManager.h>

namespace qpidnetwork {

/***************************** 状态监视处理 **************************************/
class MainLoop;
class MainLoopRunnable : public KRunnable {
public:
	MainLoopRunnable(MainLoop *container) {
		mContainer = container;
	}
	virtual ~MainLoopRunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->MainLoopHandle();
	}
private:
	MainLoop *mContainer;
};
/***************************** 状态监视处理 **************************************/

static MainLoop *gMainLoop = NULL;
MainLoop *MainLoop::GetMainLoop() {
	if( gMainLoop == NULL ) {
		gMainLoop = new MainLoop();
	}
	return gMainLoop;
}

MainLoop::MainLoop()
:mRunningMutex(KMutex::MutexType_Recursive) {
	// TODO Auto-generated constructor stub

	// 超时处理线程
	mpMainLoopRunnable = new MainLoopRunnable(this);

	mRunning = false;
}

MainLoop::~MainLoop() {
	// TODO Auto-generated destructor stub
	if (mpMainLoopRunnable) {
		delete mpMainLoopRunnable;
		mpMainLoopRunnable = NULL;
	}
}

bool MainLoop::Start() {
	bool bFlag = true;

	mRunningMutex.lock();
	if (mRunning) {
		Stop();
	}
	mRunning = true;

	bFlag = (mMainLoopThread.Start(mpMainLoopRunnable, "MainLoop") != 0);
	if( bFlag ) {
		// 服务启动成功
		LogAync(
				LOG_INFO,
				"MainLoop::Start, "
				"[OK] "
				")"
				);

	} else {
		// 服务启动失败
		LogAync(
				LOG_ALERT,
				"MainLoop::Start, "
				"[Fail] "
				")"
				);
		Stop();
	}
	mRunningMutex.unlock();

	return bFlag;
}

void MainLoop::Stop(int sign_no) {
	LogAync(
			LOG_INFO,
			"MainLoop::Stop, "
			"signal:%d "
			")",
			sign_no
			);

	mRunningMutex.lock();

	if( mRunning ) {
		mRunning = false;

		mMainLoopThread.Stop();

		mCallbackMap.Lock();
		for (MainLoopCallbackMap::iterator itr = mCallbackMap.Begin(); itr != mCallbackMap.End();) {
			MainLoopObj *obj = itr->second;
			if (!obj->isExit) {
				LogAyncUnSafe(
						LOG_INFO, "MainLoop::Stop, kill -%d %d ",
						signal,
						obj->pid
						);
				kill(obj->pid, sign_no);
			}
			delete obj;
			mCallbackMap.Erase(itr++);
		}
		mCallbackMap.Unlock();
	}

	mRunningMutex.unlock();

	LogAync(
			LOG_INFO,
			"MainLoop::Stop, "
			"[OK] "
			")"
			);
}

void MainLoop::Exit(int signal) {
//	LogAyncUnSafe(
//			LOG_INFO, "MainLoop::Exit( signal:%d )", signal
//			);
//	for (MainLoopCallbackMap::iterator itr = mCallbackMap.Begin(); itr != mCallbackMap.End(); itr++) {
//		MainLoopObj *obj = itr->second;
//		if (!obj->isExit) {
//			kill(obj->pid, signal);
//		}
//	}
}

void MainLoop::WaitPid(int pid) {
	MainLoopCallbackMap::iterator itr = mCallbackMap.Find(pid);
	if (itr != mCallbackMap.End()) {
		MainLoopObj *obj = itr->second;
		obj->isExit = true;
	}
}

void MainLoop::StartWatchChild(int pid, MainLoopCallback *cb) {
	LogAync(
			LOG_INFO,
			"MainLoop::StartWatchChild, "
			"pid:%u",
			pid
			);
	MainLoopObj *obj = new MainLoopObj(pid, cb);
	mCallbackMap.Lock();
	mCallbackMap.Insert(pid, obj);
    mCallbackMap.Unlock();
}

void MainLoop::StopWatchChild(int pid) {
	LogAync(
			LOG_INFO,
			"MainLoop::StopWatchChild, "
			"pid:%u",
			pid
			);
	MainLoopObj *obj = NULL;
	mCallbackMap.Lock();
	MainLoopCallbackMap::iterator itr = mCallbackMap.Find(pid);
	if (itr != mCallbackMap.End()) {
		obj = itr->second;
		mCallbackMap.Erase(itr);
	}
    mCallbackMap.Unlock();

    if (obj) {
    	delete obj;
    }
}

void MainLoop::MainLoopHandle() {
	LogAync(
			LOG_INFO,
			"MainLoop::MainLoopHandle, "
			"[Start]"
			);

	while (mRunning) {
		bool bSleep = true;
		MainLoopObj *obj = NULL;

		mCallbackMap.Lock();
		for (MainLoopCallbackMap::iterator itr = mCallbackMap.Begin(); itr != mCallbackMap.End();) {
			obj = itr->second;
			if (obj->isExit) {
				mCallbackMap.Erase(itr++);
				bSleep = false;
				break;
			} else {
				itr++;
			}
		}
		mCallbackMap.Unlock();

		if (!bSleep && obj) {
			if (obj->cb) {
				obj->cb->OnChildExit(obj->pid);
			}
			delete obj;
			obj = NULL;
		} else {
			sleep(1);
		}
	}

	LogAync(
			LOG_INFO,
			"MainLoop::MainLoopHandle, [Exit]"
			);
}

} /* namespace qpidnetwork */
