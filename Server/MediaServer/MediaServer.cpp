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
// Common
#include <httpclient/HttpClient.h>
#include <simulatorchecker/SimulatorProtocolTool.h>
// Request
// Respond
#include <respond/BaseRespond.h>
#include <respond/BaseResultRespond.h>
// ThirdParty
#include <json/json.h>

/***************************** 线程处理 **************************************/
/**
 * 状态监视线程
 */
class StateRunnable : public KRunnable {
public:
	StateRunnable(MediaServer *container) {
		mContainer = container;
	}
	virtual ~StateRunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->StateHandle();
	}
private:
	MediaServer *mContainer;
};

/***************************** 线程处理 end **************************************/

MediaServer::MediaServer()
:mServerMutex(KMutex::MutexType_Recursive) {
	// TODO Auto-generated constructor stub
	// Http服务
	mAsyncIOServer.SetAsyncIOServerCallback(this);
	// Websocket服务
	mWSServer.SetCallback(this);
	// 处理线程
	mpStateRunnable = new StateRunnable(this);

	// 内部服务(HTTP)参数
	miPort = 0;
	miMaxClient = 0;
	miMaxHandleThread = 0;
	miMaxQueryPerThread = 0;
	miTimeout = 0;

	// 媒体流服务(WebRTC)参数
	mWebRTCPortStart = 10000;
	mWebRTCMaxClient = 10;
	mWebRTCRtp2RtmpShellFilePath = "/root/Max/webrtc/bin/rtp2rtmp.sh";

	// 日志参数
	miStateTime = 0;
	miDebugMode = 0;
	miLogLevel = 0;

	// 统计参数
	mTotal = 0;

	// 其他
	mRunning = false;
}

MediaServer::~MediaServer() {
	// TODO Auto-generated destructor stub
	Stop();

	if( mpStateRunnable ) {
		delete mpStateRunnable;
		mpStateRunnable = NULL;
	}
}

bool MediaServer::Start(const string& config) {
	if( config.length() > 0 ) {
		mConfigFile = config;

		// LoadConfig config
		if( LoadConfig() ) {
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

	LogManager::GetLogManager()->Start(miLogLevel, mLogDir);
	LogManager::GetLogManager()->SetDebugMode(miDebugMode);
	LogManager::GetLogManager()->LogSetFlushBuffer(1 * BUFFER_SIZE_1K * BUFFER_SIZE_1K);

	LogAync(
			LOG_ERR_SYS,
			"MediaServer::Start( "
			"############## MediaServer ############## "
			")"
			);

	LogAync(
			LOG_ERR_SYS,
			"MediaServer::Start( "
			"Version : %s, "
			"Build date : %s %s "
			")",
			VERSION_STRING,
			__DATE__,
			__TIME__
			);

	// 内部服务(HTTP)参数
	LogAync(
			LOG_WARNING,
			"MediaServer::Start( "
			"[内部服务(HTTP)], "
			"miPort : %d, "
			"miMaxClient : %d, "
			"miMaxHandleThread : %d, "
			"miMaxQueryPerThread : %d, "
			"miTimeout : %d, "
			"miStateTime : %d "
			")",
			miPort,
			miMaxClient,
			miMaxHandleThread,
			miMaxQueryPerThread,
			miTimeout,
			miStateTime
			);

	// 媒体流服务(WebRTC)
	LogAync(
			LOG_WARNING,
			"MediaServer::Start( "
			"[媒体流服务(WebRTC)], "
			"mWebRTCPortStart : %u, "
			"mWebRTCMaxClient : %u, "
			"mWebRTCRtp2RtmpShellFilePath : %s "
			")",
			mWebRTCPortStart,
			mWebRTCMaxClient,
			mWebRTCRtp2RtmpShellFilePath.c_str()
			);

	// 日志参数
	LogAync(
			LOG_WARNING,
			"MediaServer::Start( "
			"[日志服务], "
			"miDebugMode : %d, "
			"miLogLevel : %d, "
			"mlogDir : %s "
			")",
			miDebugMode,
			miLogLevel,
			mLogDir.c_str()
			);

	// 初始化全局属性
	HttpClient::Init();
	if( !WebRTC::GobalInit() ) {
		return false;
	}

	mServerMutex.lock();
	if( mRunning ) {
		Stop();
	}
	mRunning = true;

	// 创建HTTP server
	if( bFlag ) {
		bFlag = mAsyncIOServer.Start(miPort, miMaxClient, miMaxHandleThread);
		if( bFlag ) {
			LogAync(
					LOG_WARNING, "MediaServer::Start( event : [创建内部服务(HTTP)-成功] )"
					);

		} else {
			LogAync(
					LOG_ERR_SYS, "MediaServer::Start( event : [创建内部服务(HTTP)-失败] )"
					);
		}
	}

	if( bFlag ) {
		bFlag = mWSServer.Start(miPort + 1);
	}

	for(unsigned int i = 0, port = mWebRTCPortStart; i < mWebRTCMaxClient; i++, port +=4) {
		WebRTC *rtc = new WebRTC();
		rtc->Init(mWebRTCRtp2RtmpShellFilePath, "127.0.0.1", port, "127.0.0.1", port + 2);
		mWebRTCList.PushBack(rtc);
	}

	// 开始状态监视线程
	if( bFlag ) {
		if( mStateThread.Start(mpStateRunnable) != 0 ) {
			LogAync(
					LOG_WARNING,
					"MediaServer::Start( "
					"event : [开始状态监视线程] "
					")");
		} else {
			bFlag = false;
			LogAync(
					LOG_ERR_SYS,
					"MediaServer::Start( "
					"event : [开始状态监视线程-失败] "
					")"
					);
		}
	}

	if( bFlag ) {
		// 服务启动成功
		LogAync(
				LOG_WARNING,
				"MediaServer::Start( "
				"event : [OK] "
				")"
				);
		printf("# MediaServer start OK. \n");

	} else {
		// 服务启动失败
		LogAync(
				LOG_ERR_SYS,
				"MediaServer::Start( "
				"event : [Fail] "
				")"
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
	if( mConfigFile.length() > 0 ) {
		ConfFile conf;
		conf.InitConfFile(mConfigFile.c_str(), "");
		if ( conf.LoadConfFile() ) {
			// 基本参数
			miPort = atoi(conf.GetPrivate("BASE", "PORT", "9876").c_str());
			miMaxClient = atoi(conf.GetPrivate("BASE", "MAXCLIENT", "100000").c_str());
			miMaxHandleThread = atoi(conf.GetPrivate("BASE", "MAXHANDLETHREAD", "2").c_str());
			miMaxQueryPerThread = atoi(conf.GetPrivate("BASE", "MAXQUERYPERCOPY", "10").c_str());
			miTimeout = atoi(conf.GetPrivate("BASE", "TIMEOUT", "10").c_str());
			miStateTime = atoi(conf.GetPrivate("BASE", "STATETIME", "30").c_str());

			// 日志参数
			miLogLevel = atoi(conf.GetPrivate("LOG", "LOGLEVEL", "5").c_str());
			mLogDir = conf.GetPrivate("LOG", "LOGDIR", "log");
			miDebugMode = atoi(conf.GetPrivate("LOG", "DEBUGMODE", "0").c_str());

			bFlag = true;
		}
	}
	mConfigMutex.unlock();
	return bFlag;
}

bool MediaServer::ReloadLogConfig() {
	bool bFlag = false;
	mConfigMutex.lock();
	if( mConfigFile.length() > 0 ) {
		ConfFile conf;
		conf.InitConfFile(mConfigFile.c_str(), "");
		if ( conf.LoadConfFile() ) {
			// 基本参数
			miStateTime = atoi(conf.GetPrivate("BASE", "STATETIME", "30").c_str());

			// 日志参数
			miLogLevel = atoi(conf.GetPrivate("LOG", "LOGLEVEL", "5").c_str());
			miDebugMode = atoi(conf.GetPrivate("LOG", "DEBUGMODE", "0").c_str());

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

bool MediaServer::Stop() {
	LogAync(
			LOG_WARNING,
			"MediaServer::Stop("
			")"
			);

	mServerMutex.lock();

	if( mRunning ) {
		mRunning = false;

		// 停止监听socket
		mAsyncIOServer.Stop();
		// 停止监听Websocket
		mWSServer.Stop();
		// 停止线程
		mStateThread.Stop();
	}

	mServerMutex.unlock();

	LogAync(
			LOG_WARNING,
			"MediaServer::Stop( "
			"event : [OK] "
			")"
			);
	printf("# MediaServer stop. \n");

	LogManager::GetLogManager()->Stop();

	return true;
}

/***************************** 线程处理函数 **************************************/
void MediaServer::StateHandle() {
	unsigned int iCount = 1;
	unsigned int iStateTime = miStateTime;

	unsigned int iTotal = 0;
	double iSecondTotal = 0;

	unsigned int iMakeCallTotal = 0;

	while( IsRunning() ) {
		if ( iCount < iStateTime ) {
			iCount++;

		} else {
			iCount = 1;
			iSecondTotal = 0;

			mCountMutex.lock();
			iTotal = mTotal;

			if( iStateTime != 0 ) {
				iSecondTotal = 1.0 * iTotal / iStateTime;
			}

			mTotal = 0;
			mCountMutex.unlock();

			LogAync(
					LOG_ERR_USER,
					"MediaServer::StateHandle( "
					"event : [内部服务(HTTP)], "
					"过去%u秒共收到请求 : %u, "
					"平均收到请求 : %.1lf/秒 "
					")",
					iStateTime,
					iTotal,
					iSecondTotal
					);

			iStateTime = miStateTime;

		}
		sleep(1);
	}
}

/***************************** 线程处理函数 end **************************************/


/***************************** 内部服务(HTTP)回调 **************************************/
bool MediaServer::OnAccept(Client *client) {
	HttpParser* parser = new HttpParser();
	parser->SetCallback(this);
	parser->custom = client;
	client->parser = parser;

	LogAync(
			LOG_MSG,
			"MediaServer::OnAccept( "
			"parser : %p, "
			"client : %p "
			")",
			parser,
			client
			);

	return true;
}

void MediaServer::OnDisconnect(Client* client) {
	HttpParser* parser = (HttpParser *)client->parser;

	LogAync(
			LOG_MSG,
			"MediaServer::OnDisconnect( "
			"parser : %p, "
			"client : %p "
			")",
			parser,
			client
			);

	if( parser ) {
		delete parser;
		client->parser = NULL;
	}
}

void MediaServer::OnHttpParserHeader(HttpParser* parser) {
	Client* client = (Client *)parser->custom;

	LogAync(
			LOG_MSG,
			"MediaServer::OnHttpParserHeader( "
			"parser : %p, "
			"client : %p, "
			"path : %s "
			")",
			parser,
			client,
			parser->GetPath().c_str()
			);

	// 可以在这里处理超时任务
//	mAsyncIOServer.GetHandleCount();

	bool bFlag = HttpParseRequestHeader(parser);
	// 已经处理则可以断开连接
	if( bFlag ) {
		mAsyncIOServer.Disconnect(client);
	}
}

void MediaServer::OnHttpParserBody(HttpParser* parser) {
	Client* client = (Client *)parser->custom;

	bool bFlag = HttpParseRequestBody(parser);
	if ( bFlag ) {
		mAsyncIOServer.Disconnect(client);
	}
}

void MediaServer::OnHttpParserError(HttpParser* parser) {
	Client* client = (Client *)parser->custom;

	LogAync(
			LOG_WARNING,
			"MediaServer::OnHttpParserError( "
			"parser : %p, "
			"client : %p "
			")",
			parser,
			client
			);

	mAsyncIOServer.Disconnect(client);
}
/***************************** 内部服务(HTTP)回调 End **************************************/


/***************************** 内部服务(HTTP)接口 **************************************/
bool MediaServer::HttpParseRequestHeader(HttpParser* parser) {
	bool bFlag = true;

	if( parser->GetPath() == "/RELOADLOGCONFIG" ) {
		// 重新加载日志配置
		OnRequestReloadLogConfig(parser);

	} else if( parser->GetPath() == "/PLAY" ) {
		// 开始拉流
		OnRequestPlayStream(parser);

	} else if( parser->GetPath() == "/STOP" ) {
		// 断开流
		OnRequestStopStream(parser);

	} else {
		bFlag = false;
	}

	return bFlag;
}

bool MediaServer::HttpParseRequestBody(HttpParser* parser) {
	bool bFlag = true;

	if( parser->GetPath() == "/CALLSDP" ) {
		// 拨号
		bFlag = OnRequestCallSdp(parser);

	} else {
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

	LogAync(
			LOG_MSG,
			"MediaServer::HttpSendRespond( "
			"event : [内部服务(HTTP)-返回请求到客户端], "
			"parser : %p, "
			"client : %p, "
			"respond : %p "
			")",
			parser,
			client,
			respond
			);

	// 发送头部
	char buffer[MAX_BUFFER_LEN];
	snprintf(
			buffer,
			MAX_BUFFER_LEN - 1,
			"HTTP/1.1 200 OK\r\n"
			"Connection:Keep-Alive\r\n"
			"Content-Type:text/html; charset=utf-8\r\n"
			"\r\n"
			);
	int len = strlen(buffer);

	mAsyncIOServer.Send(client, buffer, len);

	if( respond != NULL ) {
		// 发送内容
		bool more = false;
		while( true ) {
			len = respond->GetData(buffer, MAX_BUFFER_LEN, more);
			LogAync(
					LOG_WARNING,
					"MediaServer::HttpSendRespond( "
					"event : [内部服务(HTTP)-返回请求内容到客户端], "
					"parser : %p, "
					"client : %p, "
					"respond : %p, "
					"buffer : %s "
					")",
					parser,
					client,
					respond,
					buffer
					);

			mAsyncIOServer.Send(client, buffer, len);

			if( !more ) {
				// 全部发送完成
				bFlag = true;
				break;
			}
		}
	}

	if( !bFlag ) {
		LogAync(
				LOG_WARNING,
				"MediaServer::HttpSendRespond( "
				"event : [内部服务(HTTP)-返回请求到客户端-失败], "
				"parser : %p, "
				"client : %p "
				")",
				parser,
				client
				);
	}

	return bFlag;
}
/***************************** 内部服务(HTTP)接口 end **************************************/

/***************************** 内部服务(HTTP) 回调处理 **************************************/
void MediaServer::OnRequestReloadLogConfig(HttpParser* parser) {
	LogAync(
			LOG_WARNING,
			"MediaServer::OnRequestReloadLogConfig( "
			"event : [内部服务(HTTP)-收到命令:重新加载日志配置], "
			"parser : %p "
			")",
			parser
			);
	// 重新加载日志配置
	ReloadLogConfig();

	// 马上返回数据
	BaseResultRespond respond;
	HttpSendRespond(parser, &respond);
}

void MediaServer::OnRequestPlayStream(HttpParser* parser) {
	LogAync(
			LOG_WARNING,
			"MediaServer::OnRequestPlayStream( "
			"event : [内部服务(HTTP)-收到命令:拉流], "
			"parser : %p "
			")",
			parser
			);
	// 拉流
	const string stream = parser->GetParam("stream");
	const string ip = parser->GetParam("ip");
	const string port = parser->GetParam("port");
	const string identification = ip + ":" + port;

	string url = "rtmp://192.168.88.17:19351/live/" + stream;
	string errMsg;
	bool bFlag = mRtmpStreamPool.PlayStream(url, identification, errMsg);

	// 马上返回数据
	BaseResultRespond respond;
	respond.SetParam(bFlag, errMsg.c_str());
	HttpSendRespond(parser, &respond);
}

void MediaServer::OnRequestStopStream(HttpParser* parser) {
	LogAync(
			LOG_WARNING,
			"MediaServer::OnRequestStopStream( "
			"event : [内部服务(HTTP)-收到命令:断开流], "
			"parser : %p "
			")",
			parser
			);
	// 断开流
	const string ip = parser->GetParam("ip");
	const string port = parser->GetParam("port");
	const string identification = ip + ":" + port;

	string errMsg;
	bool bFlag = mRtmpStreamPool.StopStream(identification, errMsg);

	// 马上返回数据
	BaseResultRespond respond;
	respond.SetParam(bFlag, errMsg.c_str());
	HttpSendRespond(parser, &respond);
}

bool MediaServer::OnRequestCallSdp(HttpParser* parser) {
	// 拨号
	bool bFlag = true;
	const char* body = parser->GetBody();

	LogAync(
			LOG_WARNING,
			"MediaServer::OnRequestCallSdp( "
			"event : [内部服务(HTTP)-收到命令:拨号], "
			"parser : %p "
			")",
			parser
			);

	Json::Value root;
	Json::Reader reader;

	bool bParse = reader.parse(body, root, false);
	if ( bParse ) {
		if( root.isObject() ) {
			if( root["sdp"].isString() ) {
				string sdp = root["sdp"].asString();

//				WebRTC *rtc = new WebRTC();
//				rtc->SetCallback(this);
//				rtc->Start(sdp);
			}
		}
	}

	// 马上返回数据
	BaseResultRespond respond;
	respond.SetParam(false, "");
	HttpSendRespond(parser, &respond);

	return bFlag;
}

bool MediaServer::OnRequestUndefinedCommand(HttpParser* parser) {
	LogAync(
			LOG_WARNING,
			"MediaServer::OnRequestUndefinedCommand( "
			"event : [内部服务(HTTP)-收到命令:未知命令], "
			"parser : %p "
			")",
			parser
			);
	Client* client = (Client *)parser->custom;

	// 马上返回数据
	BaseResultRespond respond;
	respond.SetParam(false, "Undefined Command.");
	HttpSendRespond(parser, &respond);

	return true;
}

void MediaServer::OnWebRTCServerSdp(WebRTC *rtc, const string& sdp) {
	LogAync(
			LOG_WARNING,
			"MediaServer::OnWebRTCServerSdp( "
			"event : [WebRTC-返回SDP-Start], "
			"rtc : %p "
			")",
			rtc
			);

	connection_hdl hdl;
	bool bFound = false;
	mWebsocketMap.Lock();
	mWebRTCMap.Lock();
	WebRTCMap::iterator itr = mWebRTCMap.Find(rtc);
	if( itr != mWebRTCMap.End() ) {
		hdl = itr->second;
		bFound = true;
	}
	mWebRTCMap.Unlock();
	mWebsocketMap.Unlock();

	LogAync(
			LOG_WARNING,
			"MediaServer::OnWebRTCServerSdp( "
			"event : [WebRTC-返回SDP], "
			"hdl : %p, "
			"rtc : %p, "
			"sdp:\n%s"
			")",
			hdl.lock().get(),
			rtc,
			sdp.c_str()
			);

	if ( bFound ) {
		Json::Value resRoot;
		Json::Value resData;
		Json::FastWriter writer;

		resRoot["id"] = 0;
		resRoot["route"] = "imRTC/sendSdpAnswerNotice";
		resRoot["errno"] = 0;
		resRoot["errmsg"] = "";

		resData["sdp"] = sdp;
		resRoot["req_data"] = resData;

		string res = writer.write(resRoot);
		mWSServer.SendText(hdl, res);
	}
}

void MediaServer::OnWebRTCStartMedia(WebRTC *rtc) {
	connection_hdl hdl;
	bool bFound = false;
	mWebsocketMap.Lock();
	mWebRTCMap.Lock();
	WebRTCMap::iterator itr = mWebRTCMap.Find(rtc);
	if( itr != mWebRTCMap.End() ) {
		hdl = itr->second;
		bFound = true;
	}
	mWebRTCMap.Unlock();
	mWebsocketMap.Unlock();

	LogAync(
			LOG_WARNING,
			"MediaServer::OnWebRTCStartMedia( "
			"event : [WebRTC-开始媒体传输], "
			"hdl : %p, "
			"rtc : %p "
			")",
			hdl.lock().get(),
			rtc
			);

	if( bFound ) {
		Json::Value resRoot;
		Json::FastWriter writer;

		resRoot["id"] = 0;
		resRoot["route"] = "imRTC/sendStartMediaNotice";
		resRoot["errno"] = 0;
		resRoot["errmsg"] = "";

		string res = writer.write(resRoot);
		mWSServer.SendText(hdl, res);
	}
}

void MediaServer::OnWebRTCError(WebRTC *rtc, WebRTCErrorType errType, const string& errMsg) {
	connection_hdl hdl;
	bool bFound = false;
	mWebsocketMap.Lock();
	mWebRTCMap.Lock();
	WebRTCMap::iterator itr = mWebRTCMap.Find(rtc);
	if( itr != mWebRTCMap.End() ) {
		hdl = itr->second;
		bFound = true;
	}
	mWebRTCMap.Unlock();
	mWebsocketMap.Unlock();

	LogAync(
			LOG_WARNING,
			"MediaServer::OnWebRTCError( "
			"event : [WebRTC-出错], "
			"hdl : %p, "
			"rtc : %p, "
			"errType : %u, "
			"errMsg : %s "
			")",
			hdl.lock().get(),
			rtc,
			errType,
			errMsg.c_str()
			);

	if( bFound ) {
		Json::Value resRoot;
		Json::FastWriter writer;

		resRoot["id"] = 0;
		resRoot["route"] = "imRTC/sendErrorNotice";
		resRoot["errno"] = 0;
		resRoot["errmsg"] = errMsg;

		string res = writer.write(resRoot);
		mWSServer.SendText(hdl, res);

		mWSServer.Disconnect(hdl);
	}
}

void MediaServer::OnWebRTCClose(WebRTC *rtc) {
	connection_hdl hdl;
	bool bFound = false;
	mWebsocketMap.Lock();
	mWebRTCMap.Lock();
	WebRTCMap::iterator itr = mWebRTCMap.Find(rtc);
	if( itr != mWebRTCMap.End() ) {
		hdl = itr->second;
		bFound = true;
	}
	mWebRTCMap.Unlock();
	mWebsocketMap.Unlock();

	LogAync(
			LOG_WARNING,
			"MediaServer::OnWebRTCClose( "
			"event : [WebRTC-断开], "
			"hdl : %p, "
			"rtc : %p "
			")",
			hdl.lock().get(),
			rtc
			);

	if( bFound ) {
		mWSServer.Disconnect(hdl);
	}
}

void MediaServer::OnWSOpen(WSServer *server, connection_hdl hdl) {
	LogAync(
			LOG_MSG,
			"MediaServer::OnWSOpen( "
			"event : [Websocket-新连接], "
			"hdl : %p "
			")",
			hdl.lock().get()
			);
}

void MediaServer::OnWSClose(WSServer *server, connection_hdl hdl) {
	WebRTC *rtc = NULL;

	mWebsocketMap.Lock();
	mWebRTCMap.Lock();
	WebsocketMap::iterator itr = mWebsocketMap.Find(hdl);
	if( itr != mWebsocketMap.End() ) {
		rtc = itr->second;
		mWebRTCMap.Erase(rtc);
	}
	mWebsocketMap.Erase(hdl);
	mWebRTCMap.Unlock();
	mWebsocketMap.Unlock();

	LogAync(
			LOG_MSG,
			"MediaServer::OnWSClose( "
			"event : [Websocket-断开], "
			"hdl : %p, "
			"rtc : %p "
			")",
			hdl.lock().get(),
			rtc
			);

	if( rtc ) {
		rtc->Stop();
		mWebRTCList.PushBack(rtc);
	}
}

void MediaServer::OnWSMessage(WSServer *server, connection_hdl hdl, const string& str) {
	bool bFlag = false;

	Json::Value reqRoot;
	Json::Reader reader;

	Json::Value resRoot;
	Json::Value resData = Json::Value::null;
	Json::FastWriter writer;

	bool bParse = reader.parse(str, reqRoot, false);
	if ( bParse ) {
		if( reqRoot.isObject() ) {
			resRoot["id"] = reqRoot["id"];
			resRoot["route"] = reqRoot["route"];
			resRoot["errno"] = 0;
			resRoot["errmsg"] = "";

			string route = "";
			if ( resRoot["route"].isString() ) {
				route = resRoot["route"].asString();
			}

//			LogAync(
//					LOG_MSG,
//					"MediaServer::OnWSMessage( "
//					"event : [Websocket-请求], "
//					"hdl : %p, "
//					"route : %s "
//					")",
//					hdl.lock().get(),
//					route.c_str()
//					);

			bParse = false;
			if ( reqRoot["route"].isString() ) {
				string route = reqRoot["route"].asString();
				if ( route == "imRTC/sendSdpCall" ) {
					if ( reqRoot["req_data"].isObject() ) {
						Json::Value reqData = reqRoot["req_data"];

						string stream = "";
						if( reqData["stream"].isString() ) {
							stream = reqData["stream"].asString();
						}

						string sdp = "";
						if( reqData["sdp"].isString() ) {
							sdp = reqData["sdp"].asString();
						}

						string rtmpUrl = "rtmp://192.168.88.17:19351/live/";
						rtmpUrl += stream;

						if( stream.length() > 0 && sdp.length() > 0 ) {
							WebRTC *rtc = NULL;

							mWebsocketMap.Lock();
							mWebRTCMap.Lock();

							WebsocketMap::iterator itr = mWebsocketMap.Find(hdl);
							if ( itr != mWebsocketMap.End() ) {
								rtc = itr->second;
								rtc->Stop();

							} else {
								rtc = mWebRTCList.PopFront();
								if ( rtc ) {
									rtc->SetCallback(this);
									mWebsocketMap.Insert(hdl, rtc);
									mWebRTCMap.Insert(rtc, hdl);
								}
							}

							if ( rtc ) {
								bFlag = rtc->Start(sdp, rtmpUrl);
								if ( bFlag ) {
									resData["rtmpUrl"] = rtmpUrl;
								} else {
									resRoot["errno"] = 1;
									resRoot["errmsg"] = "WebRTC Start Fail Error.";
								}

							} else {
								resRoot["errno"] = 1;
								resRoot["errmsg"] = "No More WebRTC Connection Allow Error.";
							}

							mWebRTCMap.Unlock();
							mWebsocketMap.Unlock();
						} else {
							resRoot["errno"] = 1;
							resRoot["errmsg"] = "No Stream or SDP Error.";
						}
					}
				} else if ( route == "imRTC/sendSdpUpdate" ) {
					if ( reqRoot["req_data"].isObject() ) {
						Json::Value reqData = reqRoot["req_data"];

						string sdp = "";
						if( reqData["sdp"].isString() ) {
							sdp = reqData["sdp"].asString();
						}

						if( sdp.length() > 0 ) {
							WebRTC *rtc = NULL;

							mWebsocketMap.Lock();
							mWebRTCMap.Lock();

							WebsocketMap::iterator itr = mWebsocketMap.Find(hdl);
							if ( itr != mWebsocketMap.End() ) {
								rtc = itr->second;
								rtc->UpdateCandidate(sdp);
								bFlag = true;
							} else {
								resRoot["errno"] = 1;
								resRoot["errmsg"] = "WebRTC Update Candidate Before Call Error.";
							}

							mWebRTCMap.Unlock();
							mWebsocketMap.Unlock();
						} else {
							resRoot["errno"] = 1;
							resRoot["errmsg"] = "No SDP Error.";
						}
					}
				} else {
					resRoot["errno"] = 1;
					resRoot["errmsg"] = "Unknow Command Error.";
				}
			} else {
				resRoot["errno"] = 1;
				resRoot["errmsg"] = "Unknow Command Error.";
			}
		}

	} else {
		resRoot["errno"] = 1;
		resRoot["errmsg"] = "JSON Parse Error.";
	}

	resRoot["data"] = resData;

	string res = writer.write(resRoot);
	mWSServer.SendText(hdl, res);

	if ( !bFlag ) {
		LogAync(
				LOG_WARNING,
				"MediaServer::OnWSMessage( "
				"event : [Websocket-请求出错], "
				"hdl : %p, "
				"str : %s, "
				"res : %s "
				")",
				hdl.lock().get(),
				str.c_str(),
				res.c_str()
				);

		mWSServer.Disconnect(hdl);
	}
}
/***************************** 内部服务(HTTP) 回调处理 end **************************************/
