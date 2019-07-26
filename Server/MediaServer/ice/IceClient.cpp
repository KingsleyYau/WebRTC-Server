/*
 * IceClient.cpp
 *
 *  Created on: 2019/06/28
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "IceClient.h"

// Common
#include <common/LogManager.h>
#include <common/Math.h>
#include <common/CommonFunc.h>
#include <common/Arithmetic.h>

// libnice
#include <agent.h>

// glib
#include <gio/gnetworking.h>

namespace mediaserver {
::GMainLoop* gLoop = NULL;
::GThread* gLoopThread = NULL;

static const char *CandidateTypeName[] = {"host", "srflx", "prflx", "relay"};

void* loop_thread(void *data) {
	::GMainLoop* pLoop = (GMainLoop *)data;
    g_main_loop_run(pLoop);
    return 0;
}

bool IceClient::GobalInit() {
	bool bFlag = true;

	g_networking_init();
	gLoop = g_main_loop_new(NULL, FALSE);
	gLoopThread = g_thread_new("loopThread", &loop_thread, gLoop);

	return bFlag;
}

//void IceClient::GobalStop() {
//}

void cb_closed(void *src, void *res, void *data) {
	IceClient *pClient = (IceClient *)data;
	NiceAgent *agent = NICE_AGENT((GObject *)src);
	pClient->OnClose(agent);
}

void cb_nice_recv(NiceAgent *agent, guint streamId, guint componentId, guint len, gchar *buf, gpointer data) {
	IceClient* pClient = (IceClient *)data;
	pClient->OnNiceRecv(agent, streamId, componentId, len, buf);
}

void cb_candidate_gathering_done(NiceAgent *agent, guint streamId, gpointer data) {
	IceClient* pClient = (IceClient *)data;
	pClient->OnCandidateGatheringDone(agent, streamId);
}

void cb_component_state_changed(NiceAgent *agent, guint streamId, guint componentId, guint state, gpointer data) {
	IceClient* pClient = (IceClient *)data;
	pClient->OnComponentStateChanged(agent, streamId, componentId, state);
}

void cb_new_selected_pair_full(NiceAgent* agent, guint streamId, guint componentId, NiceCandidate *lcandidate, NiceCandidate* rcandidate, gpointer data) {
	IceClient* pClient = (IceClient *)data;
	pClient->OnNewSelectedPairFull(agent, streamId, componentId, lcandidate, rcandidate);
}

IceClient::IceClient() :
		mClientMutex(KMutex::MutexType_Recursive) {
	// TODO Auto-generated constructor stub
	// Status
	mRunning = false;

	// libnice
	mpAgent = NULL;
	mStreamId = -1;
	mComponentId = -1;

	mSdp = "";
	mpIceClientCallback = NULL;
}

IceClient::~IceClient() {
	// TODO Auto-generated destructor stub
	Stop();
}

bool IceClient::Start() {
	bool bFlag = false;

	LogAync(
			LOG_MSG,
			"IceClient::Start( "
			"this : %p "
			")",
			this
			);

	mClientMutex.lock();
	if( mRunning ) {
		Stop();
	}

	mRunning = true;

	mpAgent = nice_agent_new(g_main_loop_get_context(gLoop), NICE_COMPATIBILITY_RFC5245);

    /**
     * https://nice.freedesktop.org/libnice/NiceAgent.html
     *
     * stun-max-retransmissions:
     * 设置CREATE_PERMISSION后, 尝试发送Send Indication的次数, 默认为7次
     * 每次发送Send Indication的时间间隔会翻倍, 初始为200ms
     *
     * stun-initial-timeout:初始值
     *
     */
	// 被动呼叫, controlling-mode为0
    g_object_set(mpAgent, "controlling-mode", 0, NULL);
    // 允许使用turn
    g_object_set(mpAgent, "ice-tcp", TRUE, NULL);
    // 强制使用turn转发
    g_object_set(mpAgent, "force-relay", TRUE, NULL);
    // 设置超时
//    g_object_set(mpAgent, "stun-reliable-timeout", 20000, NULL);
    g_object_set(mpAgent, "stun-max-retransmissions", 10, NULL);
    // NAT网关不支持UPNP, 禁用
    g_object_set(mpAgent, "upnp", FALSE,  NULL);
    // 保持心跳
    g_object_set(mpAgent, "keepalive-conncheck", TRUE, NULL);

    g_object_set(mpAgent, "stun-server", "192.168.88.133", NULL);
    g_object_set(mpAgent, "stun-server-port", 3478, NULL);

    // 绑定本地IP
    NiceAddress addrLocal;
    nice_address_init(&addrLocal);
    nice_address_set_from_string(&addrLocal, "192.168.88.133");
    nice_agent_add_local_address(mpAgent, &addrLocal);

    g_signal_connect(mpAgent, "candidate-gathering-done", G_CALLBACK(cb_candidate_gathering_done), this);
    g_signal_connect(mpAgent, "component-state-changed", G_CALLBACK(cb_component_state_changed), this);
    g_signal_connect(mpAgent, "new-selected-pair-full", G_CALLBACK(cb_new_selected_pair_full), this);

    guint componentId = 1;
	guint streamId = nice_agent_add_stream(mpAgent, componentId);
	if ( streamId != 0 ) {
		mStreamId = streamId;
		mComponentId = componentId;

		bFlag &= nice_agent_set_relay_info(mpAgent, streamId, componentId, "192.168.88.133", 3478, "MaxServer", "123", NICE_RELAY_TYPE_TURN_TCP);
		bFlag &= nice_agent_set_stream_name(mpAgent, streamId, "audio");
		bFlag &= nice_agent_attach_recv(mpAgent, streamId, componentId, g_main_loop_get_context(gLoop), cb_nice_recv, this);
		bFlag &= nice_agent_gather_candidates(mpAgent, streamId);

		bFlag = true;
	}

	LogAync(
			LOG_MSG,
			"IceClient::Start( "
			"this : %p, "
			"[%s], "
			"streamId : %u, "
			"componentId : %u "
			")",
			this,
			bFlag?"OK":"Fail",
			streamId,
			componentId
			);

	mClientMutex.unlock();

	return bFlag;
}

void IceClient::Stop() {
	mClientMutex.lock();
	if( mRunning ) {
		LogAync(
				LOG_MSG,
				"IceClient::Stop( "
				"this : %p "
				")",
				this
				);

		if (mpAgent) {
			mCond.lock();
			mRunning = false;
			nice_agent_close_async(mpAgent, (GAsyncReadyCallback)cb_closed, this);
			mCond.wait();
			mCond.unlock();
			g_object_unref(mpAgent);
			mpAgent = NULL;
		} else {
			mRunning = false;
		}

		LogAync(
				LOG_MSG,
				"IceClient::Stop( "
				"this : %p, "
				"[OK] "
				")",
				this
				);
	}
	mClientMutex.unlock();
}

void IceClient::SetCallback(IceClientCallback *callback) {
	mpIceClientCallback = callback;
}

void IceClient::SetRemoteSdp(const string& sdp) {
	mSdp = sdp;
}

int IceClient::SendData(const void *data, unsigned int len) {
	int sendSize = 0;
	sendSize = nice_agent_send(mpAgent, mStreamId, mComponentId, len, (const gchar *)data);
	return sendSize;
}

const string& IceClient::GetLocalAddress() {
	return mLocalAddress;
}

const string& IceClient::GetRemoteAddress() {
	return mRemoteAddress;
}

bool IceClient::ParseRemoteSdp(unsigned int streamId) {
	bool bFlag = false;

    gchar *ufrag = NULL;
    gchar *pwd = NULL;
    GSList *plist = nice_agent_parse_remote_stream_sdp(mpAgent, streamId, mSdp.c_str(), &ufrag, &pwd);
    if ( ufrag && pwd && g_slist_length(plist) > 0 ) {
        NiceCandidate *c = (NiceCandidate *)g_slist_nth(plist, 0)->data;
        bFlag = nice_agent_set_remote_credentials(mpAgent, streamId, ufrag, pwd);
        if( !bFlag ) {
    		LogAync(
    				LOG_WARNING,
    				"IceClient::ParseRemoteSdp( "
    				"this : %p, "
    				"[nice_agent_set_remote_credentials Fail], "
    				"ufrag : %s, "
    				"pwd : %s "
    				")",
    				this,
    				ufrag,
    				pwd
    				);
        }

        if( bFlag ) {
        	bFlag = (nice_agent_set_remote_candidates(mpAgent, streamId, 1, plist) >= 1);
        }

        if( !bFlag ) {
    		LogAync(
    				LOG_WARNING,
    				"IceClient::ParseRemoteSdp( "
    				"this : %p, "
    				"[nice_agent_set_remote_candidates Fail], "
    				"ufrag : %s, "
    				"pwd : %s "
    				")",
    				this,
    				ufrag,
    				pwd
    				);
        }
    }

	LogAync(
			LOG_MSG,
			"IceClient::ParseRemoteSdp( "
			"this : %p, "
			"ufrag : %s, "
			"pwd : %s "
			")",
			this,
			ufrag,
			pwd
			);

    if ( ufrag ) {
    	g_free(ufrag);
    }
    if ( pwd ) {
    	g_free(pwd);
    }

    return bFlag;
}

void IceClient::OnClose(::NiceAgent *agent) {
	LogAync(
			LOG_MSG,
			"IceClient::OnClose( "
			"this : %p "
			")",
			this
			);

	mCond.lock();
	if( mRunning ) {
		if( mpIceClientCallback ) {
			mpIceClientCallback->OnIceClose(this);
		}
	}

	mCond.signal();
	mCond.unlock();
}

void IceClient::OnNiceRecv(::NiceAgent *agent, unsigned int streamId, unsigned int componentId, unsigned int len, char *buf) {
//	LogAync(
//			LOG_MSG,
//			"IceClient::OnNiceRecv( "
//			"this : %p, "
//			"streamId : %u, "
//			"componentId : %u, "
//			"len : %u "
//			")",
//			this,
//			streamId,
//			componentId,
//			len
//			);

	if( mpIceClientCallback ) {
		mpIceClientCallback->OnIceRecvData(this, (const char *)buf, len, streamId, componentId);
	}
}

void IceClient::OnCandidateGatheringDone(::NiceAgent *agent, unsigned int streamId) {
	gchar *localSdp = nice_agent_generate_local_sdp(agent);
	LogAync(
			LOG_MSG,
			"IceClient::OnCandidateGatheringDone( "
			"this : %p, "
			"streamId : %u, "
			"localSdp :\n%s"
			")",
			this,
			streamId,
			localSdp
			);

    gchar *ufrag = NULL;
    gchar *pwd = NULL;
    gchar ip[INET6_ADDRSTRLEN] = {0};

    bool bFlag = true;
	if ( bFlag ) {
	    bFlag = nice_agent_get_local_credentials(agent, streamId, &ufrag, &pwd);
	}
	if ( bFlag ) {
		GSList *cands = nice_agent_get_local_candidates(agent, streamId, 1);
		if( cands ) {
			NiceCandidate *local = (NiceCandidate *)g_slist_nth(cands, 0)->data;
			nice_address_to_string(&local->addr, ip);

			if( mpIceClientCallback ) {
				mpIceClientCallback->OnIceCandidateGatheringDone(this, CandidateTypeName[local->type], ip, nice_address_get_port(&local->addr), ufrag, pwd);
			}
		}
	}

	if ( ufrag ) {
		g_free(ufrag);
	}
	if ( pwd ) {
		g_free(pwd);
	}
	if ( localSdp ) {
		g_free(localSdp);
	}

	bFlag = ParseRemoteSdp(streamId);

}

void IceClient::OnComponentStateChanged(::NiceAgent *agent, unsigned int streamId, unsigned int componentId, unsigned int state) {
	LogAync(
			LOG_MSG,
			"IceClient::OnComponentStateChanged( "
			"this : %p, "
			"streamId : %u, "
			"componentId : %u, "
			"state : %s "
			")",
			this,
			streamId,
			componentId,
			nice_component_state_to_string((NiceComponentState)state)
			);

	if (state == NICE_COMPONENT_STATE_CONNECTED) {
	} else if (state == NICE_COMPONENT_STATE_FAILED) {
		nice_agent_close_async(mpAgent, (GAsyncReadyCallback)cb_closed, this);
	}
}

void IceClient::OnNewSelectedPairFull(::NiceAgent* agent, unsigned int streamId, unsigned int componentId, ::NiceCandidate *local, ::NiceCandidate* remote) {
	gchar localIp[INET6_ADDRSTRLEN] = {0};
	nice_address_to_string(&local->addr, localIp);
	gchar remoteIp[INET6_ADDRSTRLEN] = {0};
	nice_address_to_string(&remote->addr, remoteIp);

	char tmp[128];
	sprintf(tmp, "(%s)%s:%u", CandidateTypeName[local->type], localIp, nice_address_get_port(&local->addr));
	mLocalAddress = tmp;
	sprintf(tmp, "(%s)%s:%u", CandidateTypeName[remote->type], remoteIp, nice_address_get_port(&remote->addr));
	mRemoteAddress = tmp;

	char *localSdp = nice_agent_generate_local_sdp(mpAgent);
	LogAync(
			LOG_MSG,
			"IceClient::OnNewSelectedPairFull( "
			"this : %p, "
			"streamId : %u, "
			"componentId : %u, "
			"local : %s, "
			"remote : %s "
			")",
			this,
			streamId,
			componentId,
			mLocalAddress.c_str(),
			mRemoteAddress.c_str()
			);

	if( mpIceClientCallback ) {
		mpIceClientCallback->OnIceNewSelectedPairFull(this);
	}
}
} /* namespace mediaserver */
