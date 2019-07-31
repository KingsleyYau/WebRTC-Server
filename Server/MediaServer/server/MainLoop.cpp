/*
 * MainLoop.cpp
 *
 *  Created on: 2019/07/30
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "MainLoop.h"

namespace mediaserver {

static MainLoop *gMainLoop = NULL;
MainLoop *MainLoop::GetMainLoop() {
	if( gMainLoop == NULL ) {
		gMainLoop = new MainLoop();
	}
	return gMainLoop;
}

MainLoop::MainLoop() {
	// TODO Auto-generated constructor stub

}

MainLoop::~MainLoop() {
	// TODO Auto-generated destructor stub
}

void MainLoop::Call(int pid) {
	mCallbackMap.Lock();
	MainLoopCallbackMap::iterator itr = mCallbackMap.Find(pid);
	if ( itr != mCallbackMap.End() ) {
		MainLoopCallback *cb = itr->second;
		cb->OnChildExit(pid);
		mCallbackMap.Erase(itr);
	}
	mCallbackMap.Unlock();
}

void MainLoop::StartWatchChild(int pid, MainLoopCallback *cb) {
	mCallbackMap.Lock();
	mCallbackMap.Insert(pid, cb);
    mCallbackMap.Unlock();
}

void MainLoop::StopWatchChild(int pid) {
	mCallbackMap.Lock();
	MainLoopCallbackMap::iterator itr = mCallbackMap.Find(pid);
	if ( itr != mCallbackMap.End() ) {
		mCallbackMap.Erase(itr);
	}
    mCallbackMap.Unlock();
}

} /* namespace mediaserver */
