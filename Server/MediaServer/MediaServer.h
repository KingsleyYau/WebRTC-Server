/*
 * MediaServer.h
 *	WebRTC媒体网关服务
 *  Created on: 2019-6-13
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef MEDIASERVER_H_
#define MEDIASERVER_H_

// Common
#include <common/LogManager.h>
#include <common/ConfFile.hpp>
#include <common/KSafeMap.h>
#include <common/TimeProc.hpp>
#include <common/StringHandle.h>
#include <common/CommonFunc.h>

// ThirdParty
#include <json/json.h>

// Base Server
#include <server/AsyncIOServer.h>

// Websocket
#include <websocket/WSServer.h>

// Request/Respond
#include <parser/HttpParser.h>
#include <request/IRequest.h>
#include <respond/IRespond.h>

// RtmpStreamPool
#include <rtmp/RtmpStreamPool.h>

// WebRTC
#include <webrtc/WebRTC.h>

// ErrorCode
#include <ErrCode.h>

using namespace mediaserver;

// STL
#include <map>
#include <list>
using namespace std;

#define VERSION_STRING "1.0.0"
#define REQUEST_TIME_OUT_MS 30000

// 在线连接对象
struct MediaClient {
	MediaClient() {
		Reset();
	}

	void Reset() {
		rtc = NULL;
		connected = false;
		connectTime = 0;
		callTime = 0;
		startMediaTime = 0;
	}

	MediaClient& operator=(const MediaClient& item) {
		hdl = item.hdl;
		rtc = item.rtc;
		connectTime = item.connectTime;
		callTime = item.callTime;
		startMediaTime = item.startMediaTime;
		return *this;
	}

	bool IsTimeout() {
		bool bFlag = false;

		if ( connectTime != 0 && callTime == 0 ) {
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

	WebRTC *rtc;
	long long connectTime;
	long long callTime;
	long long startMediaTime;
};

// 在线连接列表, 因为2个Map是同时使用, 所以只需用WebRTCMap的锁
typedef KSafeMap<WebRTC*, MediaClient*> WebRTCMap;
typedef KSafeMap<connection_hdl, MediaClient*, std::owner_less<connection_hdl> > WebsocketMap;
// 空闲的WebRTC列表
typedef KSafeList<WebRTC *> WebRTCList;
// 空闲的MediaClient列表
typedef KSafeList<MediaClient *> MediaClientList;

class TimeoutCheckRunnable;
class StateRunnable;
class MediaServer :
		public AsyncIOServerCallback,
		public HttpParserCallback,
		public WebRTCCallback,
		public WSServerCallback
{
	friend class TimeoutCheckRunnable;
	friend class StateRunnable;

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
	void OnRequestStopStream(HttpParser* parser);
	bool OnRequestUndefinedCommand(HttpParser* parser);
	/***************************** 内部服务(HTTP), 命令回调 **************************************/

	/***************************** WebRTCCallback **************************************/
	void OnWebRTCServerSdp(WebRTC *rtc, const string& sdp);
	void OnWebRTCStartMedia(WebRTC *rtc);
	void OnWebRTCError(WebRTC *rtc, WebRTCErrorType errType, const string& errMsg);
	void OnWebRTCClose(WebRTC *rtc);
	/***************************** WebRTCCallback **************************************/

	/***************************** WSServerCallback **************************************/
	void OnWSOpen(WSServer *server, connection_hdl hdl, const string& addr);
	void OnWSClose(WSServer *server, connection_hdl hdl, const string& addr);
	void OnWSMessage(WSServer *server, connection_hdl hdl, const string& str);
	/***************************** WSServerCallback **************************************/

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
	void GetErrorObject(Json::Value &resErrorNo, Json::Value &resErrorMsg, RequestErrorType errType);

private:
	/***************************** 内部服务(HTTP)参数 **************************************/
	// 监听端口
	short miPort;
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
	unsigned short mWebRTCPortStart;
	// 最大媒体流转发数
	unsigned int mWebRTCMaxClient;

	// 执行转发RTMP的脚本
	string mWebRTCRtp2RtmpShellFilePath;
	// 执行转发RTMP的地址
	string mWebRTCRtp2RtmpBaseUrl;

	// DTLS证书路径
	string mWebRTCDtlsCertPath;
	string mWebRTCDtlsKeyPath;

	// 需要绑定的本地IP
	string mWebRTCLocalIp;
	// STUN服务器IP
	string mStunServerIp;
	// TURN用户名
	string mTurnUserName;
	// TURN密码
	string mTurnPassword;
	/***************************** 媒体流服务(WebRTC)参数 **************************************/



	/***************************** 信令服务(Websocket)参数 **************************************/
	// 监听端口
	short miWebsocketPort;
	// 最大连接数
	unsigned int miWebsocketMaxClient;
	/***************************** 信令服务(Websocket)参数 **************************************/



	/***************************** 日志参数 **************************************/
	// 日志等级
	int miLogLevel;
	// 日志路径
	string mLogDir;
	// 是否debug模式
	int miDebugMode;
	/***************************** 日志参数 **************************************/



	/***************************** 状态监视线程 **************************************/
	// 状态监视线程
	StateRunnable* mpStateRunnable;
	KThread mStateThread;

	/***************************** 状态监视线程 **************************************/



	/***************************** 超时处理线程 **************************************/
	// 超时处理线程
	TimeoutCheckRunnable* mpTimeoutCheckRunnable;
	KThread mTimeoutCheckThread;

	/***************************** 超时处理线程 **************************************/



	/***************************** 统计参数 **************************************/
	// 统计请求总数
	unsigned int mTotal;
	KMutex mCountMutex;

	// 监听线程输出间隔
	unsigned int miStateTime;

	/***************************** 统计参数 **************************************/



	/***************************** 运行参数 **************************************/
	// 是否运行
	KMutex mServerMutex;
	bool mRunning;

	// 配置文件
	KMutex mConfigMutex;
	string mConfigFile;

	// 内部服务(HTTP)
	AsyncIOServer mAsyncIOServer;

	// 内部Rtmp流管理
	RtmpStreamPool mRtmpStreamPool;

	// Websocket服务
	WSServer mWSServer;

	// Websocket与WebRTC关联
	WebRTCMap mWebRTCMap;
	WebsocketMap mWebsocketMap;
	// 可用的WebRTC Object
	WebRTCList mWebRTCList;

	// 可用的MediaClient
	MediaClientList mMediaClientList;
	/***************************** 运行参数 end **************************************/
};

#endif /* MEDIASERVER_H_ */
