/*
 * IceClient.h
 * ICE控制器, 管理网络穿透
 *  Created on: 2019/06/28
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef ICE_ICECLIENT_H_
#define ICE_ICECLIENT_H_

#include <common/KMutex.h>
#include <common/KCond.h>

#include <include/ErrCode.h>

// glib
#include <glib-object.h>
#include <gio/gio.h>
#include <gio/gnetworking.h>

#include <string>
#include <vector>
using namespace std;

typedef struct _NiceAgent NiceAgent;
typedef struct _NiceCandidate NiceCandidate;
namespace mediaserver {
class IceClient;
class IceClientCallback {
public:
	virtual ~IceClientCallback(){};
	virtual void OnIceCandidateGatheringFail(IceClient *ice, RequestErrorType errType) = 0;
	virtual void OnIceCandidateGatheringDone(IceClient *ice, const string& ip, unsigned int port, vector<string> candList, const string& ufrag, const string& pwd) = 0;
	virtual void OnIceNewSelectedPairFull(IceClient *ice) = 0;
	virtual void OnIceConnected(IceClient *ice) = 0;
	virtual void OnIceRecvData(IceClient *ice, const char *data, unsigned int size, unsigned int streamId, unsigned int componentId) = 0;
	virtual void OnIceFail(IceClient *ice) = 0;
	virtual void OnIceClose(IceClient *ice) = 0;
};

class IceClient {
	friend void cb_closed(::GObject *src, ::GAsyncResult *res, ::gpointer user_data);
//	friend void cb_closed(void *src, void *res, void *data);
	friend void cb_nice_recv(::NiceAgent *agent, unsigned int streamId, unsigned int componentId, unsigned int len, char *buf, void *data);
	friend void cb_candidate_gathering_done(::NiceAgent *agent, unsigned int streamId, void* data);
	friend void cb_component_state_changed(::NiceAgent *agent, unsigned int streamId, unsigned int componentId, unsigned int state, void *data);
	friend void cb_new_selected_pair_full(::NiceAgent* agent, unsigned int streamId, unsigned int componentId, ::NiceCandidate *lcandidate, ::NiceCandidate* rcandidate, void* data);
	friend void cb_stream_removed_actually(::NiceAgent *agent, unsigned int streamId, void* data);

public:
	IceClient();
	virtual ~IceClient();

public:
	static bool GobalInit(
			const string& stunServerIp,
			const string& localIp,
			bool useShareSecret,
			const string& turnUserName,
			const string& turnPassword,
			const string& turnShareSecret
			);

public:
	void SetCallback(IceClientCallback *callback);
	void SetRemoteSdp(const string& sdp);

public:
	bool Start(const string& name, bool bControlling = false);
	void Stop();
	int SendData(const void *data, unsigned int len);

public:
	const string& GetLocalAddress();
	const string& GetRemoteAddress();
	bool IsConnected();

private:
	void OnClose(::NiceAgent *agent);
	void OnNiceRecv(::NiceAgent *agent, unsigned int streamId, unsigned int componentId, unsigned int len, char *buf);
	void OnCandidateGatheringDone(::NiceAgent *agent, unsigned int streamId);
	void OnComponentStateChanged(::NiceAgent *agent, unsigned int streamId, unsigned int componentId, unsigned int state);
	void OnNewSelectedPairFull(::NiceAgent* agent, unsigned int streamId, unsigned int componentId, ::NiceCandidate *local, ::NiceCandidate* remote);
	void OnStreamRemovedActually(::NiceAgent *agent, unsigned int streamId);

private:
	bool ParseRemoteSdp(unsigned int streamId);

private:
	// Status
	KMutex mClientMutex;
	bool mRunning;

	KMutex mParamMutex;
	bool mIceGatheringDone;

	::NiceAgent *mpAgent;
	string mSdp;
	IceClientCallback *mpIceClientCallback;

	unsigned int mStreamId;
	unsigned int mComponentId;

	string mLocalAddress;
	string mRemoteAddress;

	KCond mCloseCond;
	KCond mStreamRemoveCond;

	bool mbControlling;

	// 最后一次错误码
	RequestErrorType mLastErrorCode;
};

} /* namespace mediaserver */

#endif /* ICE_ICECLIENT_H_ */
