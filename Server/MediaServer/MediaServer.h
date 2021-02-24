/*
 * MediaServer.h
 *	WebRTC媒体网关服务
 *  Created on: 2019-6-13
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef MEDIASERVER_H_
#define MEDIASERVER_H_

#include <uuid/uuid.h>

#include <Version.h>

// Common
#include <common/LogManager.h>
#include <common/ConfFile.hpp>
#include <common/KSafeMap.h>
#include <common/KSafeList.h>
#include <common/TimeProc.hpp>
#include <common/StringHandle.h>
#include <common/CommonFunc.h>
// ThirdParty
#include <json/json.h>
// Http Client
#include <httpclient/HttpClient.h>
// Base Server
#include <server/AsyncIOServer.h>
// Websocket
#include <websocket/WSServer.h>
// Request/Respond
#include <parser/HttpParser.h>
#include <request/IRequest.h>
#include <respond/IRespond.h>
// WebRTC
#include <webrtc/WebRTC.h>
// ErrorCode
#include <include/ErrCode.h>
#include <include/ForkNotice.h>

using namespace mediaserver;

// STL
#include <map>
#include <list>
using namespace std;

#define REQUEST_TIME_OUT_MS 60000

// 在线连接对象
struct MediaClient {
	MediaClient() {
		Reset();
	}

	void Reset() {
		rtc = NULL;
		connected = false;
		addr = "";
		id = 0;
		connectTime = 0;
		callTime = 0;
		startMediaTime = 0;
		logined = false;
		uuid = "";
		userAgent = "";
		extParam = "";
	}

	MediaClient& operator=(const MediaClient& item) {
		hdl = item.hdl;
		rtc = item.rtc;
		connected = item.connected;
		addr = item.addr;
		id = item.id;
		connectTime = item.connectTime;
		callTime = item.callTime;
		startMediaTime = item.startMediaTime;
		logined = item.logined;
		uuid = item.uuid;
		userAgent = item.userAgent;
		extParam = item.extParam;
		return *this;
	}

	bool IsTimeout() {
		bool bFlag = false;

		if ( connectTime != 0 && callTime == 0 && !logined ) {
			long long currentTime = getCurrentTime();
			long long ms = currentTime - connectTime;
			if ( ms > REQUEST_TIME_OUT_MS ) {
				bFlag = true;
			}
		}

		return bFlag;
	}

	connection_hdl hdl;
	bool connected;
	string addr;
	int id;

	WebRTC *rtc;
	long long connectTime;
	long long callTime;
	long long startMediaTime;
	bool logined;

	string uuid;
	string userAgent;
	string extParam;
};

enum ExtRequestType {
	ExtRequestTypeUnknow = 0,
	ExtRequestTypeLogin,
	ExtRequestTypeLogout,
};

// 在线连接对象
struct ExtRequestItem {
	ExtRequestItem() {
		uuid = "";
		type = ExtRequestTypeUnknow;
	}

	ExtRequestType type;
	string uuid;
	string extParam;
	Json::Value reqRoot;
};
// 在线连接列表, 因为2个Map是同时使用, 所以只需用WebRTCMap的锁
typedef KSafeMap<WebRTC*, MediaClient*> WebRTCMap;
typedef KSafeMap<connection_hdl, MediaClient*, std::owner_less<connection_hdl> > WebsocketMap;
// 在线的用户列表
typedef KSafeMap<string, MediaClient*> MediaClientMap;
// 空闲的WebRTC列表
typedef KSafeList<WebRTC *> WebRTCList;
// 空闲的MediaClient列表
typedef KSafeList<MediaClient *> MediaClientList;
// 外部请求队列
typedef KSafeList<ExtRequestItem *> ExtRequestList;

class StateRunnable;
class TimeoutCheckRunnable;
class ExtRequestRunnable;
class RecycleRunnable;

class MediaServer :
		public AsyncIOServerCallback,
		public HttpParserCallback,
		public WebRTCCallback,
		public WSServerCallback,
		public ForkNotice
{
	friend class StateRunnable;
	friend class TimeoutCheckRunnable;
	friend class ExtRequestRunnable;
	friend class RecycleRunnable;

public:
	MediaServer();
	virtual ~MediaServer();

	/**
	 * 启动WebRTC媒体网关服务
	 * @param config 配置文件路径
	 */
	bool Start(const string& config);
	/**
	 * 停止WebRTC媒体网关服务
	 */
	bool Stop();
	/**
	 * 是否正在运行
	 */
	bool IsRunning();
	/**
	 * 清除进程信息
	 */
	void Exit(int signal);

	/***************************** 内部服务(HTTP), 命令回调 **************************************/
	// AsyncIOServerCallback
	bool OnAccept(Client *client);
	void OnDisconnect(Client* client);

	// HttpParserCallback
	void OnHttpParserHeader(HttpParser* parser);
	void OnHttpParserBody(HttpParser* parser);
	void OnHttpParserError(HttpParser* parser);

	// HttpHandler
	void OnRequestReloadLogConfig(HttpParser* parser);
	bool OnRequestUndefinedCommand(HttpParser* parser);
	/***************************** 内部服务(HTTP), 命令回调 **************************************/

	/***************************** WebRTCCallback **************************************/
	void OnWebRTCServerSdp(WebRTC *rtc, const string& sdp, WebRTCMediaType type);
	void OnWebRTCStartMedia(WebRTC *rtc);
	void OnWebRTCError(WebRTC *rtc, RequestErrorType errType, const string& errMsg);
	void OnWebRTCClose(WebRTC *rtc);
	/***************************** WebRTCCallback **************************************/

	/***************************** WSServerCallback **************************************/
	void OnWSOpen(WSServer *server, connection_hdl hdl, const string& addr, const string& userAgent);
	void OnWSClose(WSServer *server, connection_hdl hdl);
	void OnWSMessage(WSServer *server, connection_hdl hdl, const string& str);
	/***************************** WSServerCallback **************************************/

	void OnForkBefore();
	void OnForkParent();
	void OnForkChild();

private:
	/**
	 * 启动服务
	 */
	bool Start();
	/**
	 * 加载配置
	 */
	bool LoadConfig();
	/**
	 * 重新读取日志等级
	 */
	bool ReloadLogConfig();


	/***************************** 定时任务 **************************************/
	/**
	 * 状态监视线程处理
	 */
	void StateHandle();

	/**
	 * 超时线程处理
	 */
	void TimeoutCheckHandle();

	/**
	 * 外部请求线程处理
	 */
	void ExtRequestHandle();

	/**
	 * 回收资源线程处理
	 */
	void RecycleHandle();
	/***************************** 定时任务 **************************************/



	/***************************** 内部服务(HTTP)接口 **************************************/
	/**
	 * 内部服务(HTTP), 解析请求, 仅解析头部
	 */
	bool HttpParseRequestHeader(HttpParser* parser);

	/**
	 * 内部服务(HTTP), 解析请求, 需要解析参数
	 */
	bool HttpParseRequestBody(HttpParser* parser);

	/**
	 * 内部服务(HTTP), 发送请求响应
	 * @param parser		请求
	 * @param respond		响应实例
	 * @return true:发送成功/false:发送失败
	 */
	bool HttpSendRespond(
			HttpParser* parser,
			IRespond* respond
			);
	/***************************** 内部服务(HTTP)接口 **************************************/

private:
	/**
	 * 获取错误信息结构体
	 */
	void GetErrorObject(Json::Value &resErrorNo, Json::Value &resErrorMsg, RequestErrorType errType, const string msg = "");
	/**
	 * 外部同步在线状态
	 */
	bool HandleExtForceSync(HttpClient* httpClient);
	/**
	 * 外部登录
	 */
	void HandleExtLogin(HttpClient* httpClient, ExtRequestItem *item);
	/**
	 * 外部注销
	 */
	void HandleExtLogout(HttpClient* httpClient, ExtRequestItem *item);
	/**
	 * 外部状态改变接口
	 */
	bool SendExtSetStatusRequest(HttpClient* httpClient, bool isLogin, const string& param);

private:
	/***************************** 内部服务(HTTP)参数 **************************************/
	// 监听端口
	int miPort;
	// 最大连接数
	int miMaxClient;
	// 处理线程数目
	int miMaxHandleThread;
	// 每条线程处理任务速度(个/秒)
	int miMaxQueryPerThread;
	// 请求超时(秒)
	unsigned int miTimeout;
	/***************************** 内部服务(HTTP)参数 **************************************/



	/***************************** 媒体流服务(WebRTC)参数 **************************************/
	// 媒体流转发起始端口
	int mWebRTCPortStart;
	// 最大媒体流转发数
	unsigned int mWebRTCMaxClient;

	// 执行转发RTMP的脚本
	string mWebRTCRtp2RtmpShellFilePath;
	// 执行转发RTMP的地址
	string mWebRTCRtp2RtmpBaseUrl;
	// 执行转发RTMP的地址(全录制)
	string mWebRTCRtp2RtmpBaseRecordUrl;
	// 执行转发RTP的脚本
	string mWebRTCRtmp2RtpShellFilePath;
	// 执行转发RTP的地址
	string mWebRTCRtmp2RtpBaseUrl;
	// 是否等待视频帧才开始转发
	bool mWebRTCVSync;

	// DTLS证书路径
	string mWebRTCDtlsCertPath;
	string mWebRTCDtlsKeyPath;

	// 需要绑定的本地IP
	string mWebRTCLocalIp;
	// STUN服务器IP
	string mStunServerIp;
	// STUN服务器外网IP
	string mStunServerExtIp;
	// 是否使用共享密钥
	bool mbTurnUseSecret;
	// TURN用户名
	string mTurnUserName;
	// TURN密码
	string mTurnPassword;
	// TURN共享密钥
	string mTurnShareSecret;
	// TURN共享密钥客户端有效时间(秒)
	unsigned int mTurnClientTTL;
	/***************************** 媒体流服务(WebRTC)参数 **************************************/



	/***************************** 信令服务(Websocket)参数 **************************************/
	// 监听端口
	int miWebsocketPort;
	// 最大连接数
	unsigned int miWebsocketMaxClient;
	// 外部上下线校验接口路径(空则不开启)
	string mExtSetStatusPath;
	// 外部同步在线列表接口路径(空则不开启)
	string mExtSyncStatusPath;
	// 外部同步在线列表失败重试最长时间(默认为10秒)
	unsigned int miExtSyncStatusMaxTime;
	// 外部同步在线列表失败重试时间(初始化为1秒, 递增至最大值)
	unsigned int miExtSyncStatusTime;
	/***************************** 信令服务(Websocket)参数 **************************************/



	/***************************** 日志参数 **************************************/
	// 日志等级
	int miLogLevel;
	// 日志路径
	string mLogDir;
	// 是否debug模式
	int miDebugMode;
	/***************************** 日志参数 **************************************/



	/***************************** 统计参数 **************************************/
	// 统计请求总数
	unsigned int mTotal;
	KMutex mCountMutex;

	// 监听线程输出间隔
	unsigned int miStateTime;
	/***************************** 统计参数 **************************************/



	/***************************** 定时任务线程 **************************************/
	// 状态监视线程
	StateRunnable* mpStateRunnable;
	KThread mStateThread;

	// 超时处理线程
	TimeoutCheckRunnable* mpTimeoutCheckRunnable;
	KThread mTimeoutCheckThread;

	// 外部登录校验线程
	ExtRequestRunnable* mpExtRequestRunnable;
	KThread mExtRequestThread;

	// 资源回收线程
	RecycleRunnable* mpRecycleRunnable;
	KThread mRecycleThread;
	/***************************** 定时任务线程 **************************************/



	/***************************** 运行参数 **************************************/
	// 是否运行
	KMutex mServerMutex;
	bool mRunning;
	string mPidFilePath;

	// 配置文件
	KMutex mConfigMutex;
	string mConfigFile;

	// 内部服务(HTTP)
	AsyncIOServer mAsyncIOServer;

	// Websocket服务
	WSServer mWSServer;

	// Websocket与WebRTC关联
	WebRTCMap mWebRTCMap;
	WebsocketMap mWebsocketMap;
	MediaClientMap mMediaClientMap;
	// 可用的WebRTC Object
	WebRTCList mWebRTCList;
	// 待回收的WebRTC列表
	WebRTCList mWebRTCRecycleList;
	// 可用的MediaClient
	MediaClientList mMediaClientList;
	// 外部请求队列
	ExtRequestList mExtRequestList;

	// 是否需要强制同步在线列表
	bool mbForceExtSync;
	/***************************** 运行参数 end **************************************/
};

#endif /* MEDIASERVER_H_ */
