/*
 * MainLoop.h
 *
 *  Created on: 2019/07/30
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef SERVER_MAINLOOP_H_
#define SERVER_MAINLOOP_H_

#include <common/KSafeMap.h>

namespace mediaserver {

class MainLoopCallback {
public:
	virtual ~MainLoopCallback(){};
	virtual void OnChildExit(int pid) = 0;
};

typedef KSafeMap<int, MainLoopCallback *> MainLoopCallbackMap;
class MainLoop {
public:
	static MainLoop *GetMainLoop();

public:
	MainLoop();
	virtual ~MainLoop();

	void Call(int pid);
	void StartWatchChild(int pid, MainLoopCallback *cb);
	void StopWatchChild(int pid);

private:
	MainLoopCallbackMap mCallbackMap;
};

} /* namespace mediaserver */

#endif /* SERVER_MAINLOOP_H_ */
