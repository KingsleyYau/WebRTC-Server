/*
 * MediaServer.cpp
 *
 *  Created on: 2019-6-13
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include "MediaServer.h"
// System
#include <sys/syscall.h>
#include <algorithm>
// Include
#include <include/CommonHeader.h>
// Common
#include <common/CommonFunc.h>
#include <httpclient/HttpClient.h>
#include <crypto/Crypto.h>
#include <simulatorchecker/SimulatorProtocolTool.h>
// Request
// Respond
#include <respond/BaseRespond.h>
#include <respond/BaseRawRespond.h>
#include <respond/BaseResultRespond.h>
#include <respond/UserListRespond.h>

namespace qpidnetwork {

/***************************** 状态监视处理 **************************************/
class StateRunnable : public KRunnable {
public:
	StateRunnable(MediaServer *container) {
		this->container = container;
	}
	virtual ~StateRunnable() {
		container = NULL;
	}
protected:
	void onRun() {
		container->StateHandle();
	}
private:
	MediaServer *container;
};

/***************************** 状态监视处理 **************************************/

/***************************** 超时处理 **************************************/
class TimeoutCheckRunnable:public KRunnable {
public:
	TimeoutCheckRunnable(MediaServer *container) {
		this->container = container;
	}
	virtual ~TimeoutCheckRunnable() {
		container = NULL;
	}
protected:
	void onRun() {
		container->TimeoutCheckHandle();
	}
private:
	MediaServer *container;
};

/***************************** 超时处理 **************************************/

/***************************** 外部登录处理 **************************************/
class ExtRequestRunnable:public KRunnable {
public:
	ExtRequestRunnable(MediaServer *container, ExtRequestList *requestList) {
		this->container = container;
		this->requestList = requestList;
	}
	virtual ~ExtRequestRunnable() {
		container = NULL;
	}
protected:
	void onRun() {
		container->ExtRequestHandle(requestList);
	}
private:
	MediaServer *container;
	ExtRequestList *requestList;
};

/***************************** 外部登录处理 **************************************/
/***************************** 回收处理 **************************************/
class RecycleRunnable:public KRunnable {
public:
	RecycleRunnable(MediaServer *container) {
		this->container = container;
	}
	virtual ~RecycleRunnable() {
		container = NULL;
	}
protected:
	void onRun() {
		container->RecycleHandle();
	}
private:
	MediaServer *container;
};
/***************************** 回收处理 **************************************/

MediaServer::MediaServer()
:mServerMutex(KMutex::MutexType_Recursive),
 mExtRequestMutex(KMutex::MutexType_Recursive) {
	// TODO Auto-generated constructor stub
	// Http服务
	mAsyncIOServer.SetAsyncIOServerCallback(this);
	// Websocket服务
	mWSServer.SetCallback(this);
	// 状态监视线程
	mpStateRunnable = new StateRunnable(this);
	// 超时处理线程
	mpTimeoutCheckRunnable = new TimeoutCheckRunnable(this);
//	// 外部请求线程
//	mpExtRequestRunnable = new ExtRequestRunnable(this);
	mpExtRequestThreads = NULL;
	mpExtRequestRunnables = NULL;
	mpExtRequestLists = NULL;
	// 资源回收线程
	mpRecycleRunnable = new RecycleRunnable(this);

	// 内部服务(HTTP)参数
	miPort = 0;
	miMaxClient = 0;
	miMaxHandleThread = 0;
	miMaxQueryPerThread = 0;
	miTimeout = 0;

	// 媒体流服务(WebRTC)参数
	mWebRTCPortStart = 10000;
	mWebRTCMaxClient = 10;
	mWebRTCRtp2RtmpShellFilePath = "";
	mbTurnUseSecret = false;
	mTurnUserName = "";
	mTurnPassword = "";
	mTurnShareSecret = "";
	mTurnClientTTL = 600;
	mWebRTCVSync = true;

	// 日志参数
	miStateTime = 0;
	miDebugMode = 0;
	miLogLevel = 0;

	// 信令服务(Websocket)参数
	miWebsocketPort = 0;
	miWebsocketMaxClient = 0;
	mbForceExtSync = true;
	miExtSyncStatusMaxTime = 10;
	miExtSyncStatusTime = 1;
	miExtSyncLastTime = 0;

	miExtRequestThreadCount = 0;

	// 统计参数
	mTotal = 0;
	// 其他
	mRunning = false;
	mNeedStop = false;
}

MediaServer::~MediaServer() {
	// TODO Auto-generated destructor stub
	Stop();

	if (mpStateRunnable) {
		delete mpStateRunnable;
		mpStateRunnable = NULL;
	}

	if (mpTimeoutCheckRunnable) {
		delete mpTimeoutCheckRunnable;
		mpTimeoutCheckRunnable = NULL;
	}

	if (mpRecycleRunnable) {
		delete mpRecycleRunnable;
		mpRecycleRunnable = NULL;
	}

//	if (mpExtRequestRunnable) {
//		delete mpExtRequestRunnable;
//		mpExtRequestRunnable = NULL;
//	}
}

bool MediaServer::Start(const string& config) {
	if (config.length() > 0) {
		mConfigFile = config;

		// LoadConfig config
		if (LoadConfig()) {
			return Start();

		} else {
			printf("# MediaServer can not load config file exit. \n");
		}

	} else {
		printf("# No config file can be use exit. \n");
	}

	return false;
}

bool MediaServer::Start() {
	bool bFlag = true;

	pid_t pid = getpid();
	srand(0);

	LogManager::GetLogManager()->Start(miLogLevel, mLogDir);
	LogManager::GetLogManager()->SetDebugMode(miDebugMode);
	LogManager::GetLogManager()->LogSetFlushBuffer(1 * BUFFER_SIZE_1K * BUFFER_SIZE_1K);

	LogAync(
			LOG_ALERT,
			"MediaServer::Start, "
			"############## MediaServer ############## "
			);

	LogAync(
			LOG_ALERT,
			"MediaServer::Start, "
			"Version:%s, "
			"Build date:%s %s",
			VERSION_STRING,
			__DATE__,
			__TIME__
			);

	LogAync(
			LOG_ALERT,
			"%s",
			Banner4Log()
			);

	LogAync(
			LOG_ALERT,
			"MediaServer::Start, "
			"[运行参数], "
			"pid:%d, "
			"mPidFilePath:%s, "
			"cpu:%ld",
			pid,
			mPidFilePath.c_str(),
			sysconf(_SC_NPROCESSORS_ONLN)
			);
	if (mPidFilePath.length() > 0) {
		remove(mPidFilePath.c_str());

		char cmd[1024];
		snprintf(cmd, sizeof(cmd) - 1, "echo %d > %s", pid, mPidFilePath.c_str());
		system(cmd);
	}

	// 内部服务(HTTP)参数
	LogAync(
			LOG_WARN,
			"MediaServer::Start, "
			"[内部服务(HTTP)], "
			"miPort:%d, "
			"miMaxClient:%d, "
			"miMaxHandleThread:%d, "
			"miMaxQueryPerThread:%d, "
			"miTimeout:%d, "
			"miStateTime:%d",
			miPort,
			miMaxClient,
			miMaxHandleThread,
			miMaxQueryPerThread,
			miTimeout,
			miStateTime
			);

	// 媒体流服务(WebRTC)
	LogAync(
			LOG_WARN,
			"MediaServer::Start, "
			"[媒体流服务(WebRTC)], "
			"mWebRTCPortStart:%u, "
			"mWebRTCMaxClient:%u, "
			"mWebRTCRtp2RtmpShellFilePath:%s, "
			"mWebRTCRtp2RtmpBaseUrl:%s, "
			"mWebRTCRtp2RtmpBaseRecordUrl:%s, "
			"mWebRTCRtmp2RtpShellFilePath:%s, "
			"mWebRTCRtmp2RtpBaseUrl:%s, "
			"mWebRTCDtlsCertPath:%s, "
			"mWebRTCDtlsKeyPath:%s, "
			"mWebRTCLocalIp:%s, "
			"mStunServerIp:%s, "
			"mStunServerExtIp:%s, "
			"mbTurnUseSecret:%u, "
			"mTurnUserName:%s, "
			"mTurnPassword:%s, "
			"mTurnShareSecret:%s, "
			"mTurnClientTTL:%u",
			mWebRTCPortStart,
			mWebRTCMaxClient,
			mWebRTCRtp2RtmpShellFilePath.c_str(),
			mWebRTCRtp2RtmpBaseUrl.c_str(),
			mWebRTCRtp2RtmpBaseRecordUrl.c_str(),
			mWebRTCRtmp2RtpShellFilePath.c_str(),
			mWebRTCRtmp2RtpBaseUrl.c_str(),
			mWebRTCDtlsCertPath.c_str(),
			mWebRTCDtlsKeyPath.c_str(),
			mWebRTCLocalIp.c_str(),
			mStunServerIp.c_str(),
			mStunServerExtIp.c_str(),
			mbTurnUseSecret,
			mTurnUserName.c_str(),
			mTurnPassword.c_str(),
			mTurnShareSecret.c_str(),
			mTurnClientTTL
			);

	// 信令服务(Websocket)
	LogAync(
			LOG_WARN,
			"MediaServer::Start, "
			"[信令服务(Websocket)], "
			"miWebsocketPort:%u, "
			"miWebsocketMaxClient:%u, "
			"miExtRequestThreadCount:%u",
			miWebsocketPort,
			miWebsocketMaxClient,
			miExtRequestThreadCount
			);

	// 日志参数
	LogAync(
			LOG_WARN,
			"MediaServer::Start, "
			"[日志服务], "
			"miDebugMode:%d, "
			"miLogLevel:%d, "
			"mlogDir:%s",
			miDebugMode,
			miLogLevel,
			mLogDir.c_str()
			);

	// 初始化全局属性
	HttpClient::Init();
	if (!WebRTC::GobalInit(mWebRTCDtlsCertPath, mWebRTCDtlsKeyPath, mStunServerIp, mWebRTCLocalIp, mbTurnUseSecret, mTurnUserName, mTurnPassword, mTurnShareSecret)) {
		printf("# MediaServer(WebRTC) start Fail. \n");
		return false;
	}
	WebRTC::SetDropAudioBeforeVideo(mWebRTCVSync);

	mServerMutex.lock();
	if (mRunning) {
		Stop();
	}
	mRunning = true;

	// 启动子进程监听
	if (bFlag) {
		bFlag = MainLoop::GetMainLoop()->Start();
		if (bFlag) {
			LogAync(
					LOG_NOTICE, "MediaServer::Start, event:[启动监听子进程循环-OK]"
					);
		} else {
			LogAync(
					LOG_ALERT, "MediaServer::Start, event:[启动监听子进程循环-Fail]"
					);
			printf("# MediaServer(Loop) start Fail. \n");
		}
	}

	// 启动HTTP服务
	if (bFlag) {
		bFlag = mAsyncIOServer.Start(miPort, miMaxClient, miMaxHandleThread);
		if (bFlag) {
			LogAync(
					LOG_NOTICE, "MediaServer::Start, event:[创建内部服务(HTTP)-OK]"
					);
		} else {
			LogAync(
					LOG_ALERT, "MediaServer::Start, event:[创建内部服务(HTTP)-Fail]"
					);
			printf("# MediaServer(HTTP) start Fail. \n");
		}
	}

	// 启动Websocket服务
	if (bFlag) {
		bFlag = mWSServer.Start(miWebsocketPort, miWebsocketMaxClient);
		if (bFlag) {
			LogAync(
					LOG_NOTICE, "MediaServer::Start, event:[创建内部服务(Websocket)-OK]"
					);
		} else {
			LogAync(
					LOG_ALERT, "MediaServer::Start,event:[创建内部服务(Websocket)-Fail]"
					);
			printf("# MediaServer(Websocket) start Fail. \n");
		}

		// Websocket最大连接数
		for(unsigned int i = 0; i < miWebsocketMaxClient; i++) {
			MediaClient *client = new MediaClient();
			mMediaClientList.PushBack(client);
		}
	}

	// 启动WebRTC服务
	if (bFlag) {
		// WebRTC最大转发数
		for(unsigned int i = 0, port = mWebRTCPortStart; i < mWebRTCMaxClient; i++, port +=4) {
			WebRTC *rtc = new WebRTC();
			rtc->SetCallback(this, this);
			rtc->Init(
					mWebRTCRtp2RtmpShellFilePath,
					mWebRTCRtmp2RtpShellFilePath,
					"127.0.0.1", port,
					"127.0.0.1", port + 2,
					"127.0.0.1", port
					);
			mWebRTCList.PushBack(rtc);
		}

		if (mRecycleThread.Start(mpRecycleRunnable, "Recycle") != 0) {
			LogAync(
					LOG_NOTICE,
					"MediaServer::Start, "
					"event:[启动资源回收线程-OK]"
					);
		} else {
			bFlag = false;
			LogAync(
					LOG_ALERT,
					"MediaServer::Start, "
					"event:[启动资源回收线程-Fail]"
					);
			printf("# MediaServer(Recycle Thread) start Fail. \n");
		}
	}

	// 启动外部请求线程
	if (bFlag && miExtRequestThreadCount > 0) {
		// 创建队列
		mpExtRequestLists = new ExtRequestList*[miExtRequestThreadCount];
		mpExtRequestRunnables = new ExtRequestRunnable*[miExtRequestThreadCount];
		// 创建处理线程
		mpExtRequestThreads = new KThread*[miExtRequestThreadCount];
		for (int i = 0; i < miExtRequestThreadCount; i++) {
			mpExtRequestLists[i] = new ExtRequestList();
			mpExtRequestRunnables[i] = new ExtRequestRunnable(this, mpExtRequestLists[i]);
			mpExtRequestThreads[i] = new KThread();
			mpExtRequestThreads[i]->Start(mpExtRequestRunnables[i], "ExtRequest_" + to_string(i));
		}
		LogAync(
				LOG_NOTICE,
				"MediaServer::Start, "
				"event:[启动外部请求线程-OK]"
				);
//		if (mExtRequestThread.Start(mpExtRequestRunnable, "ExtRequest") != 0) {
//			LogAync(
//					LOG_NOTICE,
//					"MediaServer::Start, "
//					"event:[启动外部请求线程-OK] "
//					")");
//		} else {
//			bFlag = false;
//			LogAync(
//					LOG_ALERT,
//					"MediaServer::Start, "
//					"event:[启动外部请求线程-Fail] "
//					")"
//					);
//			printf("# MediaServer(ExtRequest Thread) start Fail. \n");
//		}

		// 启动超时处理线程
		if (bFlag) {
			if (mTimeoutCheckThread.Start(mpTimeoutCheckRunnable, "Timeout") != 0) {
				LogAync(
						LOG_NOTICE,
						"MediaServer::Start, "
						"event:[启动超时处理线程-OK]"
						);
			} else {
				bFlag = false;
				LogAync(
						LOG_ALERT,
						"MediaServer::Start, "
						"event:[启动超时处理线程-Fail]"
						);
				printf("# MediaServer(Timeout Thread) start Fail. \n");
			}
		}
	}

	// 启动状态监视线程
	if (bFlag) {
		if (mStateThread.Start(mpStateRunnable, "State") != 0) {
			LogAync(
					LOG_NOTICE,
					"MediaServer::Start, "
					"event:[启动状态监视线程-OK] "
					);
		} else {
			bFlag = false;
			LogAync(
					LOG_ALERT,
					"MediaServer::Start, "
					"event:[启动状态监视线程-Fail]"
					);
			printf("# MediaServer(State) start Fail. \n");
		}
	}

	if (bFlag) {
		// 服务启动成功
		LogAync(
				LOG_NOTICE,
				"MediaServer::Start, "
				"event:[OK]"
				);
		printf("# MediaServer start OK. \n");

	} else {
		// 服务启动失败
		LogAync(
				LOG_ALERT,
				"MediaServer::Start, "
				"event:[Fail]"
				);
		printf("# MediaServer start Fail. \n");
		Stop();
	}
	mServerMutex.unlock();

	return bFlag;
}

bool MediaServer::LoadConfig() {
	bool bFlag = false;
	mConfigMutex.lock();
	if (mConfigFile.length() > 0) {
		ConfFile conf;
		conf.InitConfFile(mConfigFile.c_str(), "");
		if (conf.LoadConfFile()) {
			// 基本参数
			miPort = atoi(conf.GetPrivate("BASE", "PORT", "9880").c_str());
			miMaxClient = atoi(conf.GetPrivate("BASE", "MAXCLIENT", "100").c_str());
			miMaxHandleThread = atoi(conf.GetPrivate("BASE", "MAXHANDLETHREAD", "2").c_str());
			miMaxQueryPerThread = atoi(conf.GetPrivate("BASE", "MAXQUERYPERCOPY", "10").c_str());
			miTimeout = atoi(conf.GetPrivate("BASE", "TIMEOUT", "10").c_str());
			miStateTime = atoi(conf.GetPrivate("BASE", "STATETIME", "30").c_str());
			mPidFilePath = conf.GetPrivate("BASE", "PID", "").c_str();

			// 日志参数
			miLogLevel = atoi(conf.GetPrivate("LOG", "LOGLEVEL", "5").c_str());
			mLogDir = conf.GetPrivate("LOG", "LOGDIR", "log");
			miDebugMode = atoi(conf.GetPrivate("LOG", "DEBUGMODE", "0").c_str());

			// WebRTC参数
			mWebRTCPortStart = atoi(conf.GetPrivate("WEBRTC", "WEBRTCPORTSTART", "10000").c_str());
			mWebRTCMaxClient = atoi(conf.GetPrivate("WEBRTC", "WEBRTCMAXCLIENT", "10").c_str());
			mWebRTCRtp2RtmpShellFilePath = conf.GetPrivate("WEBRTC", "RTP2RTMPSHELL", "script/rtp2rtmp.sh");
			mWebRTCRtp2RtmpBaseUrl = conf.GetPrivate("WEBRTC", "RTP2RTMPBASEURL", "");
			mWebRTCRtp2RtmpBaseRecordUrl = conf.GetPrivate("WEBRTC", "RTP2RTMPBASERECORDURL", "");
			mWebRTCRtmp2RtpShellFilePath = conf.GetPrivate("WEBRTC", "RTMP2RTPSHELL", "script/rtmp2rtp.sh");
			mWebRTCRtmp2RtpBaseUrl = conf.GetPrivate("WEBRTC", "RTMP2RTPBASEURL", "");
			mWebRTCDtlsCertPath = conf.GetPrivate("WEBRTC", "DTLSCER", "etc/webrtc_dtls.crt");
			mWebRTCDtlsKeyPath = conf.GetPrivate("WEBRTC", "DTLSKEY", "etc/webrtc_dtls.key");
			mWebRTCLocalIp = conf.GetPrivate("WEBRTC", "ICELOCALIP", "");
			mStunServerIp = conf.GetPrivate("WEBRTC", "STUNSERVERIP", "127.0.0.1");
			mStunServerExtIp = conf.GetPrivate("WEBRTC", "STUNSERVEREXTIP", "127.0.0.1");
			mStunServerExtIp = (mStunServerExtIp.length()==0)?mStunServerIp:mStunServerExtIp;
			mbTurnUseSecret = atoi(conf.GetPrivate("WEBRTC", "TURNSTATIC", "0").c_str());
			mTurnUserName = conf.GetPrivate("WEBRTC", "TURNUSERNAME", "MaxServer");
			mTurnPassword = conf.GetPrivate("WEBRTC", "TURNPASSWORD", "123");
			mTurnShareSecret = conf.GetPrivate("WEBRTC", "TURNSHARESECRET", "");
			mTurnClientTTL = atoi(conf.GetPrivate("WEBRTC", "TURNCLIENTTTL", "600").c_str());
			mWebRTCVSync = atoi(conf.GetPrivate("WEBRTC", "VSYNC", "1").c_str());

			// RTP参数
			int pli_interval_max = atoi(conf.GetPrivate("RTP", "RTP_PLI_MAX_INTERVAL", "3").c_str());
			int auto_bitrate = atoi(conf.GetPrivate("RTP", "RTP_AUTO_BITRATE", "1").c_str());
			int video_min_bitrate = atoi(conf.GetPrivate("RTP", "RTP_MIN_VIDEO_BPS", "200000").c_str());
			int video_max_bitrate = atoi(conf.GetPrivate("RTP", "RTP_MAX_VIDEO_BPS", "1000000").c_str());
			int rtp_test = atoi(conf.GetPrivate("RTP", "RTP_TEST", "0").c_str());
			RtpSession::SetGobalParam(pli_interval_max, auto_bitrate, video_min_bitrate, video_max_bitrate, rtp_test);

			// Websocket参数
			miWebsocketPort = atoi(conf.GetPrivate("WEBSOCKET", "PORT", "9881").c_str());
			miWebsocketMaxClient = atoi(conf.GetPrivate("WEBSOCKET", "MAXCLIENT", "1000").c_str());
			mExtSetStatusPath = conf.GetPrivate("WEBSOCKET", "EXTSETSTATUSPATH", "");
			mExtSyncStatusPath = conf.GetPrivate("WEBSOCKET", "EXTSYNCPATH", "");
			miExtRequestThreadCount = atoi(conf.GetPrivate("WEBSOCKET", "EXTRTHREADCOUNT", "4").c_str());

			bFlag = true;
		}
	}
	mConfigMutex.unlock();
	return bFlag;
}

bool MediaServer::ReloadLogConfig() {
	bool bFlag = false;
	mConfigMutex.lock();
	if (mConfigFile.length() > 0) {
		ConfFile conf;
		conf.InitConfFile(mConfigFile.c_str(), "");
		if (conf.LoadConfFile()) {
			// 基本参数
			miStateTime = atoi(conf.GetPrivate("BASE", "STATETIME", "30").c_str());

			// 日志参数
			miLogLevel = atoi(conf.GetPrivate("LOG", "LOGLEVEL", "5").c_str());
			miDebugMode = atoi(conf.GetPrivate("LOG", "DEBUGMODE", "0").c_str());

			// WebRTC参数
			mWebRTCRtp2RtmpShellFilePath = conf.GetPrivate("WEBRTC", "RTP2RTMPSHELL", "script/rtp2rtmp.sh");
			mWebRTCRtp2RtmpBaseUrl = conf.GetPrivate("WEBRTC", "RTP2RTMPBASEURL", "");
			mWebRTCRtp2RtmpBaseRecordUrl = conf.GetPrivate("WEBRTC", "RTP2RTMPBASERECORDURL", "");
			mWebRTCRtmp2RtpShellFilePath = conf.GetPrivate("WEBRTC", "RTMP2RTPSHELL", "script/rtmp2rtp.sh");
			mWebRTCRtmp2RtpBaseUrl = conf.GetPrivate("WEBRTC", "RTMP2RTPBASEURL", "");
			mTurnClientTTL = atoi(conf.GetPrivate("WEBRTC", "TURNCLIENTTTL", "600").c_str());

			mWebRTCLocalIp = conf.GetPrivate("WEBRTC", "ICELOCALIP", "");
			mStunServerIp = conf.GetPrivate("WEBRTC", "STUNSERVERIP", "127.0.0.1");
			mStunServerExtIp = conf.GetPrivate("WEBRTC", "STUNSERVEREXTIP", "127.0.0.1");
			mStunServerExtIp = (mStunServerExtIp.length()==0)?mStunServerIp:mStunServerExtIp;
			mbTurnUseSecret = atoi(conf.GetPrivate("WEBRTC", "TURNSTATIC", "0").c_str());
			mTurnUserName = conf.GetPrivate("WEBRTC", "TURNUSERNAME", "MaxServer");
			mTurnPassword = conf.GetPrivate("WEBRTC", "TURNPASSWORD", "123");
			mTurnShareSecret = conf.GetPrivate("WEBRTC", "TURNSHARESECRET", "");
			WebRTC::ChangeGobalSetting(mStunServerIp, mWebRTCLocalIp, mbTurnUseSecret, mTurnUserName, mTurnPassword, mTurnShareSecret);

			mWebRTCVSync = atoi(conf.GetPrivate("WEBRTC", "VSYNC", "1").c_str());
			WebRTC::SetDropAudioBeforeVideo(mWebRTCVSync);

			// RTP参数
			int pli_interval_max = atoi(conf.GetPrivate("RTP", "RTP_PLI_MAX_INTERVAL", "3").c_str());
			int auto_bitrate = atoi(conf.GetPrivate("RTP", "RTP_AUTO_BITRATE", "1").c_str());
			int video_min_bitrate = atoi(conf.GetPrivate("RTP", "RTP_MIN_VIDEO_BPS", "200000").c_str());
			int video_max_bitrate = atoi(conf.GetPrivate("RTP", "RTP_MAX_VIDEO_BPS", "1000000").c_str());
			int rtp_test = atoi(conf.GetPrivate("RTP", "RTP_TEST", "0").c_str());
			RtpSession::SetGobalParam(pli_interval_max, auto_bitrate, video_min_bitrate, video_max_bitrate, rtp_test);

			LogManager::GetLogManager()->SetLogLevel(miLogLevel);
			LogManager::GetLogManager()->SetDebugMode(miDebugMode);
			LogManager::GetLogManager()->LogSetFlushBuffer(1 * BUFFER_SIZE_1K * BUFFER_SIZE_1K);

			bFlag = true;
		}
	}
	mConfigMutex.unlock();
	return bFlag;
}

bool MediaServer::IsRunning() {
	return mRunning;
}

bool MediaServer::IsNeedStop() {
	return mNeedStop;
}

void MediaServer::Exit(int sig) {
	if (!mNeedStop) {
		pid_t pid = getpid();
		mNeedStop = true;
		LogAyncUnSafe(
				LOG_ALERT,
				"MediaServer::Exit, "
				"sig:%d, "
				"pid:%d"
				,
				sig,
				pid
				);
	}
}

bool MediaServer::Stop() {
	mServerMutex.lock();
	if (mRunning) {
		pid_t pid = getpid();
		LogAync(
				LOG_ALERT,
				"MediaServer::Stop, "
				"pid:%d, "
				"mPidFilePath:%s "
				,
				pid,
				mPidFilePath.c_str()
				);

		mRunning = false;

		// 停止监听socket
		mAsyncIOServer.Stop();
		// 停止监听Websocket
		mWSServer.StopListening();

		// 断开所有Websocket连接
		mWebRTCMap.Lock();
		for (WebsocketMap::iterator itr = mWebsocketMap.Begin(); itr != mWebsocketMap.End(); itr++) {
			MediaClient *client = itr->second;
			if (client->connected) {
				mWSServer.Close(client->hdl);
				client->connected = false;
			}
		}
		mWebRTCMap.Unlock();

		// 停止监听Websocket
		mWSServer.Stop();

		// 停止定时任务
		mStateThread.Stop();
		mTimeoutCheckThread.Stop();
	//	mExtRequestThread.Stop();
		mRecycleThread.Stop();
		if (mpExtRequestRunnables) {
			delete[] mpExtRequestRunnables;
			mpExtRequestRunnables = NULL;
		}
		if (mpExtRequestThreads) {
			for (int i = 0; i < miExtRequestThreadCount; i++) {
				mpExtRequestThreads[i]->Stop();
				delete mpExtRequestThreads[i];
				delete mpExtRequestRunnables[i];
				delete mpExtRequestLists[i];
			}

			delete[] mpExtRequestThreads;
			mpExtRequestThreads = NULL;
		}
		if (mpExtRequestLists) {
			delete[] mpExtRequestLists;
			mpExtRequestLists = NULL;
		}

		// 停止子进程监听循环
		MainLoop::GetMainLoop()->Stop();

		LogAync(
				LOG_NOTICE,
				"MediaServer::Stop, "
				"event:[OK] "
				")"
				);
		printf("# MediaServer stop OK. \n");

		if (mPidFilePath.length() > 0) {
			int ret = remove(mPidFilePath.c_str());
			mPidFilePath = "";
		}
		mNeedStop = false;
	}
	mServerMutex.unlock();

	return true;
}

/***************************** 定时任务 **************************************/
void MediaServer::StateHandle() {
	unsigned int iCount = 1;
	unsigned int iStateTime = miStateTime;

	unsigned int iTotal = 0;
	double iSecondTotal = 0;

	while( IsRunning()) {
		if (iCount < iStateTime) {
			iCount++;

		} else {
			iCount = 1;
			iSecondTotal = 0;

			mCountMutex.lock();
			iTotal = mTotal;

			mTotal = 0;
			mCountMutex.unlock();

			unsigned int iLoginCount = 0;
			unsigned int iPushCount = 0;

			mWebRTCMap.Lock();
			for (WebsocketMap::iterator itr = mWebsocketMap.Begin(); itr != mWebsocketMap.End(); itr++) {
				MediaClient *client = itr->second;
				if (client->connected && client->logined) {
					iLoginCount++;
				}
				if (client->startMediaTime > 0) {
					iPushCount++;
				}
			}
			mWebRTCMap.Unlock();

			LogAync(
					LOG_WARN,
					"MediaServer::StateHandle, "
					"event:[状态服务], "
					"过去%u秒共收到请求(Websocket):%u, "
					"当前在线(Websocket):%u, "
					"当前推流(RTC):%u",
					iStateTime,
					iTotal,
					iLoginCount,
					iPushCount
					);

			iStateTime = miStateTime;

		}
		sleep(1);
	}
}

void MediaServer::TimeoutCheckHandle() {
	HttpClient httpClient;

	while( IsRunning()) {
		mWebRTCMap.Lock();
		for (WebsocketMap::iterator itr = mWebsocketMap.Begin(); itr != mWebsocketMap.End(); itr++) {
			MediaClient *client = itr->second;
			if (client->IsTimeout()) {
				if (client->connected) {
					LogAync(
							LOG_WARN,
							"MediaServer::TimeoutCheckHandle, "
							"event:[超时处理服务:断开超时连接], "
							"hdl:%p",
							client->hdl.lock().get()
							);

					mWSServer.Disconnect(client->hdl);
					client->connected = false;
				}
			}
		}
		mWebRTCMap.Unlock();

		// 同步在线状态
		mExtRequestMutex.lock();
		HandleExtForceSync(&httpClient);
		mExtRequestMutex.unlock();

		sleep(1);
	}
}

void MediaServer::ExtRequestHandle(ExtRequestList *requestList) {
	HttpClient httpClient;
	bool bFlag = false;

	while( IsRunning()) {
		mExtRequestMutex.lock();
		if (!mbForceExtSync) {
			mExtRequestMutex.unlock();
//			ExtRequestItem *item = (ExtRequestItem *)mExtRequestList.PopFront();
			ExtRequestItem *item = (ExtRequestItem *)requestList->PopFront();
			if (item) {
				switch (item->type) {
				case ExtRequestTypeLogin:{
					HandleExtLogin(&httpClient, item);
				}break;
				case ExtRequestTypeLogout:{
					HandleExtLogout(&httpClient, item);
				}break;
				default:break;
				}
				delete item;
			} else {
				sleep(1);
			}
		} else {
			mExtRequestMutex.unlock();
			sleep(1);
		}

//		bFlag = HandleExtForceSync(&httpClient);
//		if (bFlag) {
//			miExtSyncStatusTime = 1;
//			ExtRequestItem *item = (ExtRequestItem *)mExtRequestList.PopFront();
//			if (item) {
//				switch (item->type) {
//				case ExtRequestTypeLogin:{
//					HandleExtLogin(&httpClient, item);
//				}break;
//				case ExtRequestTypeLogout:{
//					HandleExtLogout(&httpClient, item);
//				}break;
//				default:break;
//				}
//				delete item;
//			} else {
//				sleep(1);
//			}
//		} else {
//			sleep(miExtSyncStatusTime);
//			miExtSyncStatusTime++;
//			miExtSyncStatusTime = MIN(miExtSyncStatusTime, miExtSyncStatusMaxTime);
//		}
	}
}

void MediaServer::RecycleHandle() {
	HttpClient httpClient;
	bool bFlag = false;

	while( IsRunning()) {
		WebRTC *rtc = mWebRTCRecycleList.PopFront();
		if (rtc) {
			rtc->Stop();
			mWebRTCList.PushBack(rtc);
			LogAync(
					LOG_NOTICE,
					"MediaServer::RecycleHandle, "
					"rtc:%p, "
					"mWebRTCRecycleList.Size:%d, "
					"mWebRTCList.Size:%d",
					rtc,
					mWebRTCRecycleList.Size(),
					mWebRTCList.Size()
					);
		} else {
			sleep(1);
		}
	}
}
/***************************** 定时任务 **************************************/



/***************************** 内部服务(HTTP)回调 **************************************/
bool MediaServer::OnAccept(Client *client) {
	HttpParser* parser = new HttpParser();
	parser->SetCallback(this);
	parser->custom = client;
	client->parser = parser;

	LogAync(
			LOG_INFO,
			"MediaServer::OnAccept, "
			"parser:%p, "
			"client:%p",
			parser,
			client
			);

	return true;
}

void MediaServer::OnDisconnect(Client* client) {
	HttpParser* parser = (HttpParser *)client->parser;

	LogAync(
			LOG_INFO,
			"MediaServer::OnDisconnect, "
			"parser:%p, "
			"client:%p",
			parser,
			client
			);

	if (parser) {
		delete parser;
		client->parser = NULL;
	}
}

void MediaServer::OnHttpParserHeader(HttpParser* parser) {
	Client* client = (Client *)parser->custom;

	LogAync(
			LOG_INFO,
			"MediaServer::OnHttpParserHeader, "
			"parser:%p, "
			"client:%p, "
			"path:%s",
			parser,
			client,
			parser->GetPath().c_str()
			);

	// 可以在这里处理超时任务
//	mAsyncIOServer.GetHandleCount();

	bool bFlag = HttpParseRequestHeader(parser);
	// 已经处理则可以断开连接
	if (bFlag) {
		mAsyncIOServer.Disconnect(client);
	}
}

void MediaServer::OnHttpParserBody(HttpParser* parser) {
	Client* client = (Client *)parser->custom;

	bool bFlag = HttpParseRequestBody(parser);
	if (bFlag) {
		mAsyncIOServer.Disconnect(client);
	}
}

void MediaServer::OnHttpParserError(HttpParser* parser) {
	Client* client = (Client *)parser->custom;

	LogAync(
			LOG_WARN,
			"MediaServer::OnHttpParserError, "
			"parser:%p, "
			"client:%p, "
			"path:%s",
			parser,
			client,
			parser->GetPath().c_str()
			);

	mAsyncIOServer.Disconnect(client);
}
/***************************** 内部服务(HTTP)回调 End **************************************/


/***************************** 内部服务(HTTP)接口 **************************************/
bool MediaServer::HttpParseRequestHeader(HttpParser* parser) {
	bool bFlag = false;

	if (!strcasecmp(parser->GetPath().c_str(), "/reload")) {
		// 重新加载日志配置
		OnRequestReloadLogConfig(parser);
	} else if (!strcasecmp(parser->GetPath().c_str(), "/getonlineusers")) {
		// 获取在线用户列表
		OnRequestGetOnlineUsers(parser);
	} else if (!strcasecmp(parser->GetPath().c_str(), "/kickuser")) {
		OnRequestKickUser(parser);
	} else {
		// 未知命令
		bFlag = OnRequestUndefinedCommand(parser);
	}

	return bFlag;
}

bool MediaServer::HttpParseRequestBody(HttpParser* parser) {
	bool bFlag = false;

	{
		// 未知命令
		bFlag = OnRequestUndefinedCommand(parser);
	}

	return bFlag;
}

bool MediaServer::HttpSendRespond(
		HttpParser* parser,
		IRespond* respond
		) {
	bool bFlag = false;
	Client* client = (Client *)parser->custom;
	const string result = respond->Result();

	// 发送头部
	char header[MAX_BUFFER_LEN];
	snprintf(
			header,
			MAX_BUFFER_LEN - 1,
			"HTTP/1.1 200 OK\r\n"
			"Connection:Keep-Alive\r\n"
			"Content-Length: %d\r\n"
			"Content-Type: application/json; charset=utf-8\r\n"
			"\r\n",
			(int)result.length()
			);

	int len = strlen(header);
	bFlag = mAsyncIOServer.Send(client, header, len);

	if (result.length() > 0) {
		len = result.length();
		bFlag = mAsyncIOServer.Send(client, result.c_str(), len);
	}

	if (bFlag) {
		LogAync(
				LOG_INFO,
				"MediaServer::HttpSendRespond, "
				"event:[内部服务(HTTP)-返回请求到客户端], "
				"parser:%p, "
				"client:%p, "
				"respond:%p, "
				"\n%s"
				"%s",
				parser,
				client,
				respond,
				header,
				result.c_str()
				);
	} else {
		LogAync(
				LOG_WARN,
				"MediaServer::HttpSendRespond, "
				"event:[内部服务(HTTP)-返回请求到客户端-失败], "
				"parser:%p, "
				"client:%p, "
				"respond:%p, "
				"\n%s"
				"%s",
				parser,
				client,
				respond,
				header,
				result.c_str()
				);
	}

	return bFlag;
}
/***************************** 内部服务(HTTP)接口 end **************************************/

/***************************** 内部服务(HTTP) 回调处理 **************************************/
void MediaServer::OnRequestReloadLogConfig(HttpParser* parser) {
	LogAync(
			LOG_NOTICE,
			"MediaServer::OnRequestReloadLogConfig, "
			"event:[内部服务(HTTP)-收到命令:重新加载日志配置], "
			"%s",
			parser->GetPath().c_str()
			);
	// 重新加载日志配置
	ReloadLogConfig();

	// 马上返回数据
	BaseResultRespond respond;
	HttpSendRespond(parser, &respond);
}

void MediaServer::OnRequestGetOnlineUsers(HttpParser* parser) {
	list<string> users;
	mWebRTCMap.Lock();
	for(WebsocketMap::iterator itr = mWebsocketMap.Begin(); itr != mWebsocketMap.End(); itr++) {
		MediaClient *client = itr->second;
		if (client->logined) {
			string user;
			GetExtParameters(client->extParam, user);
			if (user.length() > 0) {
				users.push_back(user);
			}
		}
	}
	mWebRTCMap.Unlock();

	LogAync(
			LOG_NOTICE,
			"MediaServer::OnRequestGetOnlineUsers, "
			"event:[内部服务(HTTP)-收到命令:获取在线用户列表], "
			"%s, "
			"users.size:%d",
			parser->GetPath().c_str(),
			users.size()
			);

	// 马上返回数据
	UserListRespond respond;
	respond.SetUserList(users);
	HttpSendRespond(parser, &respond);
}

void MediaServer::OnRequestKickUser(HttpParser* parser) {
	string userId = parser->GetParam("userId");

	LogAync(
			LOG_NOTICE,
			"MediaServer::OnRequestKickUser, "
			"event:[内部服务(HTTP)-收到命令:踢出用户连接], "
			"%s, "
			"userId:%s",
			parser->GetPath().c_str(),
			userId.c_str()
			);

	BaseResultRespond respond;

	mWebRTCMap.Lock();
	MediaUserMap::iterator itr = mMediaUserMap.Find(userId);
	if (itr != mMediaUserMap.End()) {
		MediaClient *client = itr->second;
		connection_hdl hdl = client->hdl;

		Json::Value rep;
		Json::FastWriter writer;

		rep["id"] = client->id++;
		rep["route"] = "imRTC/sendKickUserNotice";
		rep["errno"] = 0;
		rep["errmsg"] = "";

		string res = writer.write(rep);
		mWSServer.SendText(hdl, res);

		respond.SetParam(client->connected);
		if (client->connected) {
			mWSServer.Disconnect(hdl);
			client->connected = false;
		}
	}
	mWebRTCMap.Unlock();

	HttpSendRespond(parser, &respond);
}

bool MediaServer::OnRequestUndefinedCommand(HttpParser* parser) {
	LogAync(
			LOG_WARN,
			"MediaServer::OnRequestUndefinedCommand, "
			"event:[内部服务(HTTP)-收到命令:未知命令], "
			"%s",
			parser->GetPath().c_str()
			);
	Client* client = (Client *)parser->custom;

	// 马上返回数据
	BaseResultRespond respond;
	respond.SetParam(false, "Undefined Command.");
	HttpSendRespond(parser, &respond);

	return true;
}

void MediaServer::OnWebRTCServerSdp(WebRTC *rtc, const string& sdp, WebRTCMediaType type) {
	connection_hdl hdl;
	const void *hdlAddr = NULL;
	bool bFound = false;

	mWebRTCMap.Lock();
	WebRTCMap::iterator itr = mWebRTCMap.Find(rtc);
	if (itr != mWebRTCMap.End()) {
		MediaClient *client = itr->second;
		hdl = client->hdl;
		hdlAddr = hdl.lock().get();
		bFound = true;

		if (bFound) {
			Json::Value rep;
			Json::Value resData;
			Json::FastWriter writer;

			rep["id"] = client->id++;
			rep["route"] = "imRTC/sendSdpAnswerNotice";
			rep["errno"] = 0;
			rep["errmsg"] = "";

			resData["sdp"] = sdp;
			rep["req_data"] = resData;

			string res = writer.write(rep);
			mWSServer.SendText(hdl, res);
		}
	}

	LogAync(
			LOG_NOTICE,
			"MediaServer::OnWebRTCServerSdp, "
			"event:[WebRTC-返回SDP], "
			"hdl:%p, "
			"rtc:%p, "
			"rtmpUrl:%s, "
			"type:%s",
			hdlAddr,
			rtc,
			rtc->GetRtmpUrl().c_str(),
			WebRTCMediaTypeString[type].c_str()
			);
	mWebRTCMap.Unlock();

}

void MediaServer::OnWebRTCStartMedia(WebRTC *rtc) {
	connection_hdl hdl;
	const void *hdlAddr = NULL;
	bool bFound = false;

	mWebRTCMap.Lock();
	WebRTCMap::iterator itr = mWebRTCMap.Find(rtc);
	if (itr != mWebRTCMap.End()) {
		MediaClient *client = itr->second;
		client->startMediaTime = getCurrentTime();
		hdl = client->hdl;
		hdlAddr = hdl.lock().get();
		bFound = true;

		if (bFound) {
			Json::Value rep;
			Json::FastWriter writer;

			rep["id"] = client->id++;
			rep["route"] = "imRTC/sendStartMediaNotice";
			rep["errno"] = 0;
			rep["errmsg"] = "";

			string res = writer.write(rep);
			mWSServer.SendText(hdl, res);
		}
	}

	LogAync(
			LOG_NOTICE,
			"MediaServer::OnWebRTCStartMedia, "
			"event:[WebRTC-开始媒体传输], "
			"hdl:%p, "
			"rtc:%p, "
			"%s, "
			"rtmpUrl:%s",
			hdlAddr,
			rtc,
			rtc->Desc().c_str(),
			rtc->GetRtmpUrl().c_str()
			);
	mWebRTCMap.Unlock();
}

void MediaServer::OnWebRTCError(WebRTC *rtc, RequestErrorType errType, const string& errMsg) {
	connection_hdl hdl;
	const void *hdlAddr = NULL;
	bool bFound = false;

	mWebRTCMap.Lock();
	WebRTCMap::iterator itr = mWebRTCMap.Find(rtc);
	if (itr != mWebRTCMap.End()) {
		MediaClient *client = itr->second;
		hdl = client->hdl;
		hdlAddr = hdl.lock().get();

		Json::Value rep;
		Json::FastWriter writer;

		rep["id"] = client->id++;
		rep["route"] = "imRTC/sendErrorNotice";
		GetErrorObject(rep["errno"], rep["errmsg"], errType, errMsg);

		LogAync(
				LOG_WARN,
				"MediaServer::OnWebRTCError, "
				"event:[WebRTC-出错], "
				"hdl:%p, "
				"rtc:%p, "
				"rtmpUrl:%s, "
				"errType:%u, "
				"errMsg:%s",
				hdlAddr,
				rtc,
				rtc->GetRtmpUrl().c_str(),
				errType,
				errMsg.c_str()
				);

		string res = writer.write(rep);
		mWSServer.SendText(hdl, res);
		if (client->connected) {
			mWSServer.Disconnect(hdl);
			client->connected = false;
		}
	}
	mWebRTCMap.Unlock();
}

void MediaServer::OnWebRTCClose(WebRTC *rtc) {
	connection_hdl hdl;
	const void *hdlAddr = NULL;
	bool bFound = false;

	mWebRTCMap.Lock();
	WebRTCMap::iterator itr = mWebRTCMap.Find(rtc);
	if (itr != mWebRTCMap.End()) {
		MediaClient *client = itr->second;
		hdl = client->hdl;
		hdlAddr = hdl.lock().get();
		bFound = true;

		LogAync(
				LOG_NOTICE,
				"MediaServer::OnWebRTCClose, "
				"event:[WebRTC-断开], "
				"hdl:%p, "
				"rtc:%p, "
				"rtmpUrl:%s",
				hdlAddr,
				rtc,
				rtc->GetRtmpUrl().c_str()
				);

		if (bFound) {
			if (client->connected) {
				mWSServer.Disconnect(hdl);
				client->connected = false;
			}
		}
	}

	mWebRTCMap.Unlock();
}

void MediaServer::OnWSOpen(WSServer *server, connection_hdl hdl, const string& addr, const string& userAgent) {
	long long currentTime = getCurrentTime();
	if (!mRunning) {
		LogAync(
				LOG_ERR,
				"MediaServer::OnWSOpen, "
				"event:[Websocket-新连接, 服务器未启动, 断开], "
				"hdl:%p, "
				"addr:%s, "
				"connectTime:%lld, "
				"userAgent:%s",
				hdl.lock().get(),
				addr.c_str(),
				currentTime,
				userAgent.c_str()
				);
		mWSServer.Disconnect(hdl);
	}

	// Create UUID
	uuid_t uuid = {0};
	uuid_generate(uuid);
	char uuid_str[64] = {0};
	uuid_unparse_upper(uuid, uuid_str);

	LogAync(
			LOG_NOTICE,
			"MediaServer::OnWSOpen, "
			"event:[Websocket-新连接], "
			"hdl:%p, "
			"addr:%s, "
			"uuid:%s, "
			"connectTime:%lld, "
			"userAgent:%s",
			hdl.lock().get(),
			addr.c_str(),
			uuid_str,
			currentTime,
			userAgent.c_str()
			);

	MediaClient *client = mMediaClientList.PopFront();
	if (client) {
		client->hdl = hdl;
		client->connectTime = currentTime;
		client->connected = true;
		client->addr = addr;
		client->uuid = uuid_str;
		client->userAgent = userAgent;

		mWebRTCMap.Lock();
		mWebsocketMap.Insert(hdl, client);
		mMediaClientMap.Insert(uuid_str, client);
		mWebRTCMap.Unlock();

	} else {
		LogAync(
				LOG_WARN,
				"MediaServer::OnWSOpen, "
				"event:[超过最大连接数, 断开连接], "
				"hdl:%p, "
				"addr:%s, "
				"uuid:%s, "
				"connectTime:%lld, "
				"userAgent:%s",
				hdl.lock().get(),
				addr.c_str(),
				uuid_str,
				currentTime,
				userAgent.c_str()
				);
		mWSServer.Disconnect(hdl);
	}
}

void MediaServer::OnWSClose(WSServer *server, connection_hdl hdl) {
	long long connectTime = getCurrentTime();
	long long currentTime = connectTime;

	if (!mRunning) {
		LogAync(
				LOG_ERR,
				"MediaServer::OnWSClose, "
				"event:[Websocket-断开, 服务器未启动], "
				"hdl:%p",
				hdl.lock().get()
				);
	}

	MediaClient *client = NULL;
	WebRTC *rtc = NULL;
	string addr = "";
	string uuid = "";
	string userAgent = "";
	string user = "";

	mWebRTCMap.Lock();
	WebsocketMap::iterator itr = mWebsocketMap.Find(hdl);
	if (itr != mWebsocketMap.End()) {
		client = itr->second;

		if (client->logined) {
			// 插入外部注销通知
			ExtRequestItem *item = new ExtRequestItem();
			item->type = ExtRequestTypeLogout;
			item->uuid = client->uuid;
			item->extParam = client->extParam;

			int i = GetExtParameters(client->extParam, user);
			ExtRequestList *requestList = mpExtRequestLists[i];
			requestList->PushBack(item);
			mMediaUserMap.Erase(user);
		}

		// Remove rtc
		addr = client->addr;
		connectTime = client->connectTime;
		userAgent = client->userAgent;
		rtc = client->rtc;
		mWebRTCMap.Erase(rtc);

		// Remove uuid
		uuid = client->uuid;
		mMediaClientMap.Erase(uuid);

		// 重置 client
		client->Reset();
	}
	mWebsocketMap.Erase(hdl);
	mWebRTCMap.Unlock();

	if (rtc) {
		mWebRTCRecycleList.PushBack(rtc);
	}

	if (client) {
		mMediaClientList.PushBack(client);
	}

	LogAync(
			LOG_NOTICE,
			"MediaServer::OnWSClose, "
			"event:[Websocket-断开], "
			"hdl:%p, "
			"rtc:%p, "
			"uuid:%s, "
			"addr:%s, "
			"user:%s, "
			"userAgent:%s, "
			"aliveTime:%lldms",
			hdl.lock().get(),
			rtc,
			uuid.c_str(),
			addr.c_str(),
			user.c_str(),
			userAgent.c_str(),
			currentTime - connectTime
			);
}

void MediaServer::OnWSMessage(WSServer *server, connection_hdl hdl, const string& str) {
	bool bFlag = false;
	bool bParse = false;

	Json::Value req;
	Json::Reader reader;

	Json::Value rep = Json::Value::null;
	Json::FastWriter writer;
	string json;

	mCountMutex.lock();
	mTotal++;
	mCountMutex.unlock();

	LogAync(
			LOG_DEBUG,
			"MediaServer::OnWSMessage, "
			"event:[Websocket-请求], "
			"hdl:%p, "
			"str:%s "
			,
			hdl.lock().get(),
			str.c_str()
			);

	// Parse Json
	bParse = reader.parse(str, req, false);
	if (bParse && mRunning) {
		if (req.isObject()) {
			rep["id"] = req["id"];
			rep["route"] = req["route"];
			rep["errno"] = RequestErrorType_None;
			rep["errmsg"] = "";

			string route = "";
			if (rep["route"].isString()) {
				route = rep["route"].asString();
			}

			if (req["route"].isString()) {
				string route = req["route"].asString();
				if (route == "imRTC/sendPing") {
					bFlag = OnWSRequestPing(req, rep);
				} else if (route == "imRTC/sendSdpCall") {
					bFlag = OnWSRequestSdpCall(req, rep, hdl);
				} else if (route == "imRTC/sendSdpPull") {
					bFlag = OnWSRequestSdpPull(req, rep, hdl);
				} else if (route == "imRTC/sendSdpUpdate") {
					bFlag = OnWSRequestSdpUpdate(req, rep, hdl);
				} else if (mExtSetStatusPath.length() > 0 && route == "imRTC/login") {
					bFlag = OnWSRequestLogin(req, rep, hdl);
				} else if (route == "imRTC/sendGetToken") {
					bFlag = OnWSRequestGetToken(req, rep, hdl);
				} else {
					GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_Request_Unknow_Command);
				}
			} else {
				GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_Request_Unknow_Command);
			}
		} else {
			GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_Request_Data_Format_Parse);
		}
	} else {
		GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_Request_Data_Format_Parse);
	}

	if (rep != Json::Value::null) {
		json = writer.write(rep);
		mWSServer.SendText(hdl, json);
	}

	if (!bFlag) {
		if (bParse) {
			LogAync(
					LOG_WARN,
					"MediaServer::OnWSMessage, "
					"event:[Websocket-请求出错], "
					"hdl:%p, "
					"req:%s, "
					"result:%s",
					hdl.lock().get(),
					str.c_str(),
					json.c_str()
					);
		}

		mWSServer.Disconnect(hdl);
	}
}

bool MediaServer::OnWSRequestPing(Json::Value req, Json::Value &rep) {
	rep["data"] = Json::Value::null;
	return true;
}

bool MediaServer::OnWSRequestSdpCall(Json::Value req, Json::Value &rep, connection_hdl hdl) {
	bool bFlag = false;
	Json::Value data = Json::Value::null;
	if (req["req_data"].isObject()) {
		Json::Value reqData = req["req_data"];

		string stream = "";
		if (reqData["stream"].isString()) {
			stream = reqData["stream"].asString();
		}

		int record = 0;
		if (reqData["record"].isInt()) {
			record = reqData["record"].asInt();
		}

		string rtmpUrl = mWebRTCRtp2RtmpBaseUrl;
		if (record == 1) {
			if (mWebRTCRtp2RtmpBaseRecordUrl.length() > 0) {
				rtmpUrl = mWebRTCRtp2RtmpBaseRecordUrl;
			}
		}

		string sdp = "";
		if (reqData["sdp"].isString()) {
			sdp = reqData["sdp"].asString();
		}

		bool ignoreMedia = false;
		if (reqData["ignoreMedia"].isBool()) {
			ignoreMedia = reqData["ignoreMedia"].asBool();
		}

		if (rtmpUrl.length() > 0) {
			rtmpUrl += "/";
			rtmpUrl += stream;
		}

		LogAync(
				LOG_NOTICE,
				"MediaServer::OnWSRequestSdpCall, "
				"event:[Websocket-请求-推流], "
				"hdl:%p, "
				"stream:%s, "
				"record:%d, "
				"ignoreMedia:%s, "
				"rtmpUrl:%s",
				hdl.lock().get(),
				stream.c_str(),
				record,
				BOOL_2_STRING(ignoreMedia),
				rtmpUrl.c_str()
				);

		if (mWebRTCRtp2RtmpShellFilePath.length() > 0 &&
				rtmpUrl.length() > 0 && stream.length() > 0 && sdp.length() > 0) {
			WebRTC *rtc = NULL;

			mWebRTCMap.Lock();
			WebsocketMap::iterator itr = mWebsocketMap.Find(hdl);
			if (itr != mWebsocketMap.End()) {
				MediaClient *client = itr->second;
				client->callTime = getCurrentTime();

				if (client->rtc) {
					rtc = client->rtc;
				} else {
					rtc = mWebRTCList.PopFront();
					if (rtc) {
						client->rtc = rtc;
						mWebRTCMap.Insert(rtc, client);
					}
				}
			}
			mWebRTCMap.Unlock();

			if (rtc) {
				bFlag = rtc->Start(sdp, rtmpUrl, false, false, ignoreMedia);
				if (bFlag) {
					data["rtmpUrl"] = rtmpUrl;
				} else {
					GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_WebRTC_Start_Fail, rtc->GetLastErrorMessage());
				}

			} else {
				GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_WebRTC_No_More_WebRTC_Connection_Allow);
				LogAync(
						LOG_WARN,
						"MediaServer::OnWSRequestSdpCall, "
						"event:[Websocket-请求-推流-失败, 超过最大推流数量], "
						"hdl:%p, "
						"stream:%s, "
						"record:%d, "
						"ignoreMedia:%s, "
						"rtmpUrl:%s",
						hdl.lock().get(),
						stream.c_str(),
						record,
						BOOL_2_STRING(ignoreMedia),
						rtmpUrl.c_str()
						);
			}

		} else {
			GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_Request_Missing_Param);
		}
	}
	rep["data"] = data;
	return bFlag;
}

bool MediaServer::OnWSRequestSdpPull(Json::Value req, Json::Value &rep, connection_hdl hdl) {
	bool bFlag = false;
	Json::Value data = Json::Value::null;
	if (req["req_data"].isObject()) {
		Json::Value reqData = req["req_data"];

		string stream = "";
		if (reqData["stream"].isString()) {
			stream = reqData["stream"].asString();
		}

		string serverId = "";
		if (reqData["server_id"].isString()) {
			serverId = reqData["server_id"].asString();
			if (serverId.length() > 0) {
				serverId = "_" + serverId;
			}
		}

		string sdp = "";
		if (reqData["sdp"].isString()) {
			sdp = reqData["sdp"].asString();
		}

		string rtmpUrl = mWebRTCRtmp2RtpBaseUrl;
		rtmpUrl += serverId;
		rtmpUrl += "/";
		rtmpUrl += stream;

		LogAync(
				LOG_NOTICE,
				"MediaServer::OnWSRequestSdpPull, "
				"event:[Websocket-请求-拉流], "
				"hdl:%p, "
				"stream:%s, "
				"serverId:%s, "
				"rtmpUrl:%s",
				hdl.lock().get(),
				stream.c_str(),
				serverId.c_str(),
				rtmpUrl.c_str()
				);

		if (mWebRTCRtmp2RtpShellFilePath.length() > 0 &&
				mWebRTCRtmp2RtpBaseUrl.length() > 0 && stream.length() > 0 && sdp.length() > 0
				) {
			WebRTC *rtc = NULL;

			mWebRTCMap.Lock();
			WebsocketMap::iterator itr = mWebsocketMap.Find(hdl);
			if (itr != mWebsocketMap.End()) {
				MediaClient *client = itr->second;
				client->callTime = getCurrentTime();

				if (client->rtc) {
					rtc = client->rtc;
				} else {
					rtc = mWebRTCList.PopFront();
					if (rtc) {
						client->rtc = rtc;
						mWebRTCMap.Insert(rtc, client);
					}
				}
			}
			mWebRTCMap.Unlock();

			if (rtc) {
				bFlag = rtc->Start(sdp, rtmpUrl, true);
				if (bFlag) {
					data["rtmpUrl"] = rtmpUrl;
				} else {
					GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_WebRTC_Start_Fail, rtc->GetLastErrorMessage());
				}

			} else {
				GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_WebRTC_No_More_WebRTC_Connection_Allow);
				LogAync(
						LOG_WARN,
						"MediaServer::OnWSRequestSdpPull, "
						"event:[Websocket-请求-拉流-失败, 超过最大推流数量], "
						"hdl:%p, "
						"stream:%s, "
						"rtmpUrl:%s",
						hdl.lock().get(),
						stream.c_str(),
						rtmpUrl.c_str()
						);
			}

		} else {
			GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_Request_Missing_Param);
		}
	}
	rep["data"] = data;

	return bFlag;
}


bool MediaServer::OnWSRequestSdpUpdate(Json::Value req, Json::Value &rep , connection_hdl hdl) {
	bool bFlag = false;
	if (req["req_data"].isObject()) {
		Json::Value reqData = req["req_data"];

		string sdp = "";
		if (reqData["sdp"].isString()) {
			sdp = reqData["sdp"].asString();
		}

		if (sdp.length() > 0) {
			WebRTC *rtc = NULL;

			mWebRTCMap.Lock();
			WebsocketMap::iterator itr = mWebsocketMap.Find(hdl);
			if (itr != mWebsocketMap.End()) {
				MediaClient *client = itr->second;
				rtc = client->rtc;
				if (rtc) {
					rtc->UpdateCandidate(sdp);
					bFlag = true;
				} else {
					GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_WebRTC_Update_Candidate_Before_Call);
				}
			} else {
				GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_Unknow_Error);
			}
			mWebRTCMap.Unlock();

		} else {
			GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_Request_Missing_Param);
		}
	}
	return bFlag;
}

bool MediaServer::OnWSRequestLogin(Json::Value req, Json::Value &rep, connection_hdl hdl) {
	bool bFlag = false;
	string param = "";
	if (req["req_data"].isObject()) {
		Json::Value reqData = req["req_data"];
		if (reqData["param"].isString()) {
			param = reqData["param"].asString();
		}
	}
	LogAync(
			LOG_NOTICE,
			"MediaServer::OnWSRequestLogin, "
			"event:[Websocket-请求-设置在线状态], "
			"hdl:%p, "
			"param:%s",
			hdl.lock().get(),
			param.c_str()
			);

	mExtRequestMutex.lock();
	if (!mbForceExtSync) {
		mExtRequestMutex.unlock();
		// Start External Login
		mWebRTCMap.Lock();
		WebsocketMap::iterator itr = mWebsocketMap.Find(hdl);
		if (itr != mWebsocketMap.End()) {
			MediaClient *client = itr->second;
			string user;
			int i = GetExtParameters(param, user);
			ExtRequestList *requestList = mpExtRequestLists[i];
			if (requestList->Size() <= 300) {
				ExtRequestItem *item = new ExtRequestItem();
				item->type = ExtRequestTypeLogin;
				item->uuid = client->uuid;
				item->req = req;
				item->extParam = param;
				requestList->PushBack(item);
				bFlag = true;
				// 清空返回
				rep = Json::Value::null;
			} else {
				GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_Request_Server_Busy);
			}
		} else {
			GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_Unknow_Error);
		}
		mWebRTCMap.Unlock();
	} else {
		mExtRequestMutex.unlock();
		GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_Request_Server_Internal_Error);
	}

	return bFlag;
}

bool MediaServer::OnWSRequestGetToken(Json::Value req, Json::Value &rep, connection_hdl hdl) {
	bool bFlag = false;
	Json::Value data = Json::Value::null;
	string userId = "";
	if (req["req_data"].isObject()) {
		Json::Value reqData = req["req_data"];
		if (reqData["user_id"].isString()) {
			userId = reqData["user_id"].asString();
		}
	}

	// 这里只需要精确到秒
	char user[1024] = {0};
	time_t timer = time(NULL);
	snprintf(user, sizeof(user) - 1, "%lu:%s", timer + mTurnClientTTL, (userId.length() > 0)?userId.c_str():"client");
//					string password = Crypto::Sha1("mediaserver12345", user);
	unsigned char sha1Pwd[EVP_MAX_MD_SIZE + 1] = {0};
	int length = Crypto::Sha1("mediaserver12345", user, sha1Pwd);
	Arithmetic art;
	string base64 = art.Base64Encode((const char *)sha1Pwd, length);

	LogAync(
			LOG_NOTICE,
			"MediaServer::OnWSRequestGetToken, "
			"event:[Websocket-请求-获取ICE配置], "
			"hdl:%p, "
			"user:%s, "
			"base64:%s, "
			"ttl:%u",
			hdl.lock().get(),
			user,
			base64.c_str(),
			mTurnClientTTL
			);

	bFlag = true;

	Json::Value iceServers = Json::Value::null;
	Json::Value urls = Json::Value::null;

	char url[1024];
	snprintf(url, sizeof(url) - 1, "turn:%s?transport=tcp", mStunServerExtIp.c_str());
	urls.append(url);
	snprintf(url, sizeof(url) - 1, "turn:%s?transport=udp", mStunServerExtIp.c_str());
	urls.append(url);

	iceServers["urls"] = urls;
	iceServers["username"] = user;
	iceServers["credential"] = base64;

	data["iceServers"].append(iceServers);
	rep["data"] = data;

	return bFlag;
}

void MediaServer::OnForkBefore() {
//	// 停止监听Websocket
//	mWSServer.OnForkBefore();
}

void MediaServer::OnForkParent() {
//	// 停止监听Websocket
//	mWSServer.OnForkParent();
}

void MediaServer::OnForkChild() {
//	// 停止监听Websocket
//	mWSServer.OnForkChild();
	// 子进程停止监听socket
	mAsyncIOServer.Close();
}

void MediaServer::GetErrorObject(Json::Value &resErrorNo, Json::Value &resErrorMsg, RequestErrorType errType, const string msg) {
	ErrObject obj = RequestErrObjects[errType];
	resErrorNo = obj.errNo;
	resErrorMsg = (msg.length() > 0)?msg:obj.errMsg;
}

bool MediaServer::HandleExtForceSync(HttpClient* httpClient) {
	bool bFlag = true;
	long long currentTime = getCurrentTime();

	if ((mExtSyncStatusPath.length() > 0) &&
			(mbForceExtSync && (currentTime - miExtSyncLastTime > 30 * 1000))) {
		bFlag = false;

		// Request HTTP
		long httpCode = 0;
		const char* res = NULL;
		int respondSize = 0;

		// Request JSON
		Json::Value req = Json::objectValue;
		Json::FastWriter writer;

		req["params"] = Json::arrayValue;
		mWebRTCMap.Lock();
		// Pop All Logout Request
		for (int i = 0; i < miExtRequestThreadCount; i++) {
			ExtRequestList *requestList = mpExtRequestLists[i];
			requestList->Lock();
			for(ExtRequestList::iterator itr = requestList->Begin(); itr != requestList->End(); itr++) {
				ExtRequestItem *item = *itr;
				if (item->type == ExtRequestTypeLogout) {
					requestList->PopValueUnSafe(itr++);
					delete item;
				}
			}
			requestList->Unlock();
		}

		// Send Sync Online List
		for(WebsocketMap::iterator itr = mWebsocketMap.Begin(); itr != mWebsocketMap.End(); itr++) {
			MediaClient *client = itr->second;
			if (client->logined) {
				req["params"].append(client->extParam);
			}
		}
		mWebRTCMap.Unlock();

		string json = writer.write(req);

		HttpEntiy httpEntiy;
		httpEntiy.SetRawData(json.c_str());

		string url = mExtSyncStatusPath;
		if (httpClient->Request(url.c_str(), &httpEntiy)) {
			httpCode = httpClient->GetRespondCode();
			httpClient->GetBody(&res, respondSize);

			if (respondSize > 0) {
				// 发送成功
				Json::Value rep;
				Json::Reader reader;
				if (reader.parse(res, rep, false)) {
					if (rep.isObject()) {
						if (rep["ret"].isInt()) {
							int errNo = rep["ret"].asInt();
							if (errNo == 1) {
								bFlag = true;
								mbForceExtSync = false;
							}
						}
					}
				}
			}
		}

		if (bFlag) {
			LogAync(
					LOG_NOTICE,
					"MediaServer::HandleExtForceSync, "
					"event:[外部同步在线状态-%s], "
					"url:%s, "
					"count:%u, "
					"res:%s",
					FLAG_2_STRING(bFlag),
					url.c_str(),
					req["params"].size(),
					res
					);
		} else {
			LogAync(
					LOG_WARN,
					"MediaServer::HandleExtForceSync, "
					"event:[外部同步在线状态-%s], "
					"url:%s, "
					"count:%u, "
					"res:%s",
					FLAG_2_STRING(bFlag),
					url.c_str(),
					req["params"].size(),
					res
					);
		}
		miExtSyncLastTime = getCurrentTime();
	}

	return bFlag;
}

void MediaServer::HandleExtLogin(HttpClient* httpClient, ExtRequestItem *item) {
	bool bFlag = false;

	// Request Param
	Json::Value req = item->req;
	string param = "";

	// Respond
	Json::Value rep;
	Json::FastWriter writer;
	rep["id"] = req["id"];
	rep["route"] = req["route"];
	rep["errno"] = RequestErrorType_None;
	rep["errmsg"] = "";

	if (req["req_data"].isObject()) {
		Json::Value reqData = req["req_data"];

		if (reqData["param"].isString()) {
			param = reqData["param"].asString();
		}

		if (param.length() > 0) {
			bFlag = SendExtSetStatusRequest(httpClient, true, param);
			if (!bFlag) {
				GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_Ext_Login_Error);
			}
		} else {
			GetErrorObject(rep["errno"], rep["errmsg"], RequestErrorType_Request_Missing_Param);
		}
	}

	bool sendLogout = false;
	// Callback to client
	mWebRTCMap.Lock();
	MediaClientMap::iterator itr = mMediaClientMap.Find(item->uuid);
	if (itr != mMediaClientMap.End()) {
		MediaClient *client = itr->second;
		if (client->connected) {
			string res = writer.write(rep);
			mWSServer.SendText(client->hdl, res);

			if (bFlag && !client->logined) {
				// 成功登录
				client->logined = true;
				client->extParam = param;

				string user;
				GetExtParameters(client->extParam, user);
				if (user.length() > 0) {
					mMediaUserMap.Insert(user, client);
				}
			} else {
				// 外部校验失败, 重复登录, 踢掉
				mWSServer.Disconnect(client->hdl);
				client->connected = false;
			}
		} else {
			LogAync(
					LOG_NOTICE,
					"MediaServer::HandleExtLogin, "
					"event:[外部登录, 客户端连接已经断开-FIN], "
					"uuid:%s, "
					"param:%s",
					item->uuid.c_str(),
					param.c_str()
					);
			if(bFlag) {
				sendLogout = true;
			}
		}
	} else {
		LogAync(
				LOG_NOTICE,
				"MediaServer::HandleExtLogin, "
				"event:[外部登录, 客户端连接已经断开-Close_Wait], "
				"uuid:%s, "
				"param:%s",
				item->uuid.c_str(),
				param.c_str()
				);
		if(bFlag) {
			sendLogout = true;
		}
	}
	mWebRTCMap.Unlock();

	if (sendLogout) {
		// 重新插入下线通知
		ExtRequestItem *logoutItem = new ExtRequestItem();
		logoutItem->type = ExtRequestTypeLogout;
		logoutItem->extParam = param;
		logoutItem->uuid = item->uuid;
		string user;
		int i = GetExtParameters(item->extParam, user);
		ExtRequestList *requestList = mpExtRequestLists[i];
		requestList->PushBack(logoutItem);
	}
}

void MediaServer::HandleExtLogout(HttpClient* httpClient, ExtRequestItem *item) {
	bool bFlag = false;
	bFlag = SendExtSetStatusRequest(httpClient, false, item->extParam);
}

bool MediaServer::SendExtSetStatusRequest(
		HttpClient* httpClient,
		bool isLogin,
		const string& param
		) {
	bool bFlag = false;

	long httpCode = 0;
	const char* res = NULL;
	int respondSize = 0;

	// Request
	Json::Value req;
	Json::FastWriter writer;
	req["param"] = param;
	req["status"] = (int)isLogin;
	string json = writer.write(req);

	HttpEntiy httpEntiy;
	httpEntiy.SetRawData(json);
	string url = mExtSetStatusPath;

	LogAync(
			LOG_NOTICE,
			"MediaServer::SendExtSetStatusRequest, "
			"event:[外部%s], "
			"url:%s, "
			"req:%s",
			isLogin?"登录":"注销",
			url.c_str(),
			json.c_str()
			);

	if (httpClient->Request(url.c_str(), &httpEntiy, false)) {
		httpCode = httpClient->GetRespondCode();
		httpClient->GetBody(&res, respondSize);

		if (respondSize > 0) {
			// 发送成功
			Json::Value rep;
			Json::Reader reader;
			if (reader.parse(res, rep, false)) {
				if (rep.isObject()) {
					if (rep["ret"].isInt()) {
						int errNo = rep["ret"].asInt();
						if (errNo == 1) {
							bFlag = true;
						}
					}
				}
			}
		}
	}

	if (httpCode != 200 || (!bFlag && !isLogin)) {
		/**
		 * 两种情况需要同步
		 * 1.下线失败
		 * 2.请求没有返回200
		 */
		mExtRequestMutex.lock();
		mbForceExtSync = true;
		mExtRequestMutex.unlock();
	}

	LogAync(
			LOG_NOTICE,
			"MediaServer::SendExtSetStatusRequest, "
			"event:[外部%s-%s], "
			"url:%s, "
			"req:%s, "
			"httpCode:%d, "
			"res:%s",
			isLogin?"登录":"注销",
			FLAG_2_STRING(bFlag),
			url.c_str(),
			json.c_str(),
			httpCode,
			res
			);

	return bFlag;
}

int MediaServer::GetExtParameters(const string& wholeLine, string& userId) {
	int result = 0;
	string key;
	string value;

	string line;
	string::size_type posSep;
	string::size_type index = 0;
	string::size_type pos;

	while (string::npos != index) {
		pos = wholeLine.find("&", index);
		if (string::npos != pos) {
			// 找到分隔符
			line = wholeLine.substr(index, pos - index);
			// 移动下标
			index = pos + 1;

		} else {
			// 是否最后一次
			if (index < wholeLine.length()) {
				line = wholeLine.substr(index, pos - index);
				// 移动下标
				index = string::npos;

			} else {
				// 找不到
				index = string::npos;
				break;
			}
		}

		posSep = line.find("=");
		if ((string::npos != posSep) && (posSep + 1 < line.length())) {
			key = line.substr(0, posSep);
			value = line.substr(posSep + 1, line.length() - (posSep + 1));

		} else {
			key = line;
		}

		if (key == "userId") {
			userId = value;
			for(auto ch:value) {
				result += ch;
			}
			if (miExtRequestThreadCount > 0) {
				result %= miExtRequestThreadCount;
			}
			break;
		}
	}
	return result;
}
}
