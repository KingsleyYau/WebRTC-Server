/*
 * MediaServer.h
 *
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

// Base Server
#include <server/AsyncIOServer.h>

// Request/Respond
#include <parser/HttpParser.h>
#include <request/IRequest.h>
#include <respond/IRespond.h>

// RtmpStreamPool
#include <rtmp/RtmpStreamPool.h>
using namespace mediaserver;

// STL
#include <map>
#include <list>
using namespace std;

#define VERSION_STRING "1.0.0"

class HttpClient;
// socket -> client
typedef KSafeMap<int, Client*> ClientMap;
class StateRunnable;
class MediaServer :
		public AsyncIOServerCallback,
		public HttpParserCallback
{

public:
	MediaServer();
	virtual ~MediaServer();

	bool Start(const string& config);
	bool Start();
	bool Stop();
	bool IsRunning();

	/***************************** 线程处理函数 **************************************/
	/**
	 * 检测状态线程处理
	 */
	void StateHandle();

	/***************************** 线程处理函数 end **************************************/

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
	void OnRequestPlayStream(HttpParser* parser);
	void OnRequestStopStream(HttpParser* parser);
	void OnRequestCallSdp(HttpParser* parser);
	void OnRequestUndefinedCommand(HttpParser* parser);
	/***************************** 内部服务(HTTP), 命令回调 end **************************************/

	/***************************** 内部服务(Freeswitch), 命令回调 **************************************/
	/***************************** 内部服务(Freeswitch), 命令回调 end **************************************/

	/***************************** 外部服务(LiveChat), 任务回调 **************************************/
	/***************************** 外部服务(LiveChat), 任务回调 end **************************************/

private:
	/**
	 * 加载配置
	 */
	bool LoadConfig();
	/**
	 * 重新读取日志等级
	 */
	bool ReloadLogConfig();

	/***************************** 内部服务接口 **************************************/
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

	/***************************** 内部服务接口 end **************************************/


	/***************************** 基本参数 **************************************/
	/**
	 * 监听端口
	 */
	short miPort;

	/**
	 * 最大连接数
	 */
	int miMaxClient;

	/**
	 * 处理线程数目
	 */
	int miMaxHandleThread;

	/**
	 * 每条线程处理任务速度(个/秒)
	 */
	int miMaxQueryPerThread;

	/**
	 * 请求超时(秒)
	 */
	unsigned int miTimeout;

	/**
	 * flash请求超时(秒)
	 */
	unsigned int miFlashTimeout;
	/***************************** 基本参数 end **************************************/


	/***************************** 日志参数 **************************************/
	/**
	 * 日志等级
	 */
	int miLogLevel;

	/**
	 * 日志路径
	 */
	string mLogDir;

	/**
	 * 是否debug模式
	 */
	int miDebugMode;
	/***************************** 日志参数 end **************************************/


	/***************************** 处理线程 **************************************/
	/**
	 * 状态监视线程
	 */
	StateRunnable* mpStateRunnable;
	KThread mStateThread;

	/***************************** 处理线程 end **************************************/


	/***************************** 统计参数 **************************************/
	/**
	 * 统计请求总数
	 */
	unsigned int mTotal;
	KMutex mCountMutex;

	/**
	 * 监听线程输出间隔
	 */
	unsigned int miStateTime;

	/***************************** 统计参数 end **************************************/


	/***************************** 运行参数 **************************************/
	/**
	 * 运行锁
	 */
	KMutex mServerMutex;
	/**
	 * 是否运行
	 */
	bool mRunning;

	/**
	 * 配置文件锁
	 */
	KMutex mConfigMutex;
	/**
	 * 配置文件
	 */
	string mConfigFile;

	/**
	 * 内部服务(HTTP)
	 */
	AsyncIOServer mAsyncIOServer;


	/***************************** 运行参数 end **************************************/

	// 内部Rtmp流管理
	RtmpStreamPool mRtmpStreamPool;
};

#endif /* MEDIASERVER_H_ */
