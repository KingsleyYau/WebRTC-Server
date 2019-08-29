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
#include <debug.h>

// glib
#include <gio/gnetworking.h>

namespace mediaserver {
::GMainLoop* gLoop = NULL;
::GThread* gLoopThread = NULL;

static const char *CandidateTypeName[] = {"host", "srflx", "prflx", "relay"};
static const char *CandidateTransportName[] = {"", "tcptype active", "tcptype passive", ""};

void* loop_thread(void *data) {
	::GMainLoop* pLoop = (GMainLoop *)data;
    g_main_loop_run(pLoop);
    return 0;
}

void* niceLogFunc(const char *logBuffer) {
//	LogAync(
//			LOG_WARNING,
//			"IceClient::niceLogFunc( "
//			"[libnice], "
//			"%s "
//			")",
//			logBuffer
//			);
	return 0;
}

static string gStunServerIp = "";
static string gLocalIp = "";
bool IceClient::GobalInit(const string& stunServerIp, const string& localIp) {
	bool bFlag = true;

	gLocalIp = localIp;
	gStunServerIp = stunServerIp;

	g_networking_init();
	gLoop = g_main_loop_new(NULL, FALSE);
	gLoopThread = g_thread_new("loopThread", &loop_thread, gLoop);

	nice_debug_enable(TRUE);
	nice_debug_set_func((NICE_LOG_FUNC_IMP)&niceLogFunc);

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
	mIceGatheringDone = false;

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
     * 发送CREATE_PERMISSION后, 尝试发送Send Indication的次数, 默认为7次, 大概为25s, (2^7*200/1000)=25s
     * 每次发送Send Indication的时间间隔会翻倍, 默认初始为200ms
     *
     * stun-initial-timeout:
     * 每次发送Send Indication的初始值, 默认为200ms
     *
     */
	// 被动呼叫, controlling-mode为0
    g_object_set(mpAgent, "controlling-mode", 1, NULL);
    // 允许使用turn
    g_object_set(mpAgent, "ice-tcp", TRUE, NULL);
    // 强制使用turn转发
    g_object_set(mpAgent, "force-relay", FALSE, NULL);
    // 设置超时
//    g_object_set(mpAgent, "stun-reliable-timeout", 7900, NULL);
    g_object_set(mpAgent, "stun-max-retransmissions", 7, NULL);
    // NAT网关不支持UPNP, 禁用
    g_object_set(mpAgent, "upnp", FALSE,  NULL);
    // 保持心跳
//    g_object_set(mpAgent, "keepalive-conncheck", TRUE, NULL);
//    g_object_set(mpAgent, "max-connectivity-checks", 5, NULL);

    // 设置STUN服务器地址
    g_object_set(mpAgent, "stun-server", gStunServerIp.c_str(), NULL);
    g_object_set(mpAgent, "stun-server-port", 3478, NULL);

    if ( gLocalIp.length() > 0 ) {
		// 绑定本地IP
		NiceAddress addrLocal;
		nice_address_init(&addrLocal);
		nice_address_set_from_string(&addrLocal, gLocalIp.c_str());
		nice_agent_add_local_address(mpAgent, &addrLocal);
    }

    g_signal_connect(mpAgent, "candidate-gathering-done", G_CALLBACK(cb_candidate_gathering_done), this);
    g_signal_connect(mpAgent, "component-state-changed", G_CALLBACK(cb_component_state_changed), this);
    g_signal_connect(mpAgent, "new-selected-pair-full", G_CALLBACK(cb_new_selected_pair_full), this);

    guint componentId = 1;
	guint streamId = nice_agent_add_stream(mpAgent, componentId);
	if ( streamId != 0 ) {
		mStreamId = streamId;
		mComponentId = componentId;

		bFlag &= nice_agent_set_relay_info(mpAgent, streamId, componentId, gStunServerIp.c_str(), 3478, "MaxClient", "123", NICE_RELAY_TYPE_TURN_TCP);
		bFlag &= nice_agent_set_stream_name(mpAgent, streamId, "video");
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
			mStreamId = -1;
			mComponentId = -1;
		} else {
			mRunning = false;
		}

		mIceGatheringDone = false;

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
	mClientMutex.lock();

	mSdp = sdp;

	if ( mpAgent ) {
		NiceComponentState state = nice_agent_get_component_state(mpAgent, mStreamId, mComponentId);
//		if( state == NICE_COMPONENT_STATE_CONNECTING ) {
		if ( mIceGatheringDone && state < NICE_COMPONENT_STATE_CONNECTED ) {
			ParseRemoteSdp(mStreamId);
		}
	}

	mClientMutex.unlock();
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

	mClientMutex.lock();

	LogAync(
			LOG_MSG,
			"IceClient::ParseRemoteSdp( "
			"this : %p, "
			"sdp :\n%s"
			")",
			this,
			mSdp.c_str()
			);

    gchar *ufrag = NULL;
    gchar *pwd = NULL;
    GSList *plist = nice_agent_parse_remote_stream_sdp(mpAgent, streamId, mSdp.c_str(), &ufrag, &pwd);
	LogAync(
			LOG_MSG,
			"IceClient::ParseRemoteSdp( "
			"this : %p, "
			"plist_count : %d, "
			"ufrag : %s, "
			"pwd : %s "
			")",
			this,
			g_slist_length(plist),
			ufrag,
			pwd
			);

    if ( ufrag && pwd && g_slist_length(plist) > 0 ) {
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
        	bFlag = (nice_agent_set_remote_candidates(mpAgent, streamId, mComponentId, plist) >= 1);
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

    mClientMutex.unlock();

    if ( ufrag ) {
    	g_free(ufrag);
    }
    if ( pwd ) {
    	g_free(pwd);
    }

    if (plist) {
    	g_slist_free(plist);
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

	LogAync(
			LOG_MSG,
			"IceClient::OnClose( "
			"this : %p, "
			"[Exit] "
			")",
			this
			);
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

	if ( mpIceClientCallback ) {
		mpIceClientCallback->OnIceRecvData(this, (const char *)buf, len, streamId, componentId);
	}
}

void IceClient::OnCandidateGatheringDone(::NiceAgent *agent, unsigned int streamId) {
	gchar *localCandidate = nice_agent_generate_local_sdp(agent);
	LogAync(
			LOG_MSG,
			"IceClient::OnCandidateGatheringDone( "
			"this : %p, "
			"streamId : %u, "
			"localCandidate :\n%s"
			")",
			this,
			streamId,
			localCandidate
			);

    gchar *ufrag = NULL;
    gchar *pwd = NULL;
    gchar ip[INET6_ADDRSTRLEN] = {0};
    gchar baseip[INET6_ADDRSTRLEN] = {0};

    bool bFlag = true;
	if ( bFlag ) {
	    bFlag = nice_agent_get_local_credentials(agent, streamId, &ufrag, &pwd);
	}
	if ( bFlag ) {
		vector<string> candArray;
		string ipUse;
		unsigned int portUse = 0;
		unsigned int priority = 0xFFFFFFFF;

		char candStr[1024] = {'0'};
		GSList *cands = nice_agent_get_local_candidates(agent, streamId, 1);
		if( cands ) {
			int count = g_slist_length(cands);
			for(int i = 0; i < count; i++) {
				NiceCandidate *local = (NiceCandidate *)g_slist_nth(cands, i)->data;
				nice_address_to_string(&local->addr, ip);
				unsigned int localPort = (nice_address_get_port(&local->addr)==0)?9:nice_address_get_port(&local->addr);
				nice_address_to_string(&local->base_addr, baseip);
				unsigned int basePort = (nice_address_get_port(&local->base_addr)==0)?9:nice_address_get_port(&local->base_addr);

				if ( priority > local->priority ) {
					priority = local->priority;
					ipUse = ip;
					portUse = nice_address_get_port(&local->addr);
				}

				string transport;
				if ( strlen(CandidateTransportName[local->transport]) > 0 ) {
					transport += " ";
					transport += CandidateTransportName[local->transport];
				}
				if ( local->type == NICE_CANDIDATE_TYPE_RELAYED || local->type == NICE_CANDIDATE_TYPE_SERVER_REFLEXIVE ) {
					snprintf(candStr, sizeof(candStr) - 1,
							"a=candidate:%s 1 %s %u %s %u typ %s raddr %s rport %u%s\n",
							local->foundation,
							(local->transport == NICE_CANDIDATE_TRANSPORT_UDP)?"UDP":"TCP",
							local->priority,
							ip,
							localPort,
							CandidateTypeName[local->type],
							baseip,
							basePort,
							transport.c_str()
							);
					priority = local->priority;
					ipUse = ip;
					portUse = nice_address_get_port(&local->addr);

				} else {
					snprintf(candStr, sizeof(candStr) - 1,
							"a=candidate:%s 1 %s %u %s %u typ %s%s\n",
							local->foundation,
							(local->transport == NICE_CANDIDATE_TRANSPORT_UDP)?"UDP":"TCP",
							local->priority,
							ip,
							localPort,
							CandidateTypeName[local->type],
							transport.c_str()
							);
				}

				candArray.push_back(string(candStr));
			}

			if( mpIceClientCallback ) {
				mpIceClientCallback->OnIceCandidateGatheringDone(this, ipUse, portUse, candArray, ufrag, pwd);
			}
		}
	}

	if ( ufrag ) {
		g_free(ufrag);
	}
	if ( pwd ) {
		g_free(pwd);
	}
	if ( localCandidate ) {
		g_free(localCandidate);
	}

	mClientMutex.lock();
	mIceGatheringDone = true;
	mClientMutex.unlock();

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
		if( mpIceClientCallback ) {
			mpIceClientCallback->OnIceConnected(this);
		}
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
	sprintf(tmp, "(%s)(%s)%s:%u", (local->transport==NICE_CANDIDATE_TRANSPORT_UDP)?"udp":"tcp", CandidateTypeName[local->type], localIp, nice_address_get_port(&local->addr));
	mLocalAddress = tmp;
	sprintf(tmp, "(%s)(%s)%s:%u", (remote->transport==NICE_CANDIDATE_TRANSPORT_UDP)?"udp":"tcp", CandidateTypeName[remote->type], remoteIp, nice_address_get_port(&remote->addr));
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
