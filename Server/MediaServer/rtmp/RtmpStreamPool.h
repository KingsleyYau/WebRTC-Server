/*
 * RtmpStreamPool.h
 *
 *  Created on: Jun 13, 2019
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTMPDUMP_RTMPSTREAMPOOL_H_
#define RTMPDUMP_RTMPSTREAMPOOL_H_

#include <common/KThread.h>
#include <common/KSafeList.h>
#include <common/KSafeMap.h>
#include <common/LogManager.h>

#include <string>
using namespace std;

#include <rtmp/RtmpClient.h>
#include <rtp/RtpRawClient.h>

struct ev_io;
struct ev_loop;
namespace mediaserver {

class RtmpIORunnable;
class RtmpClient;
class RtmpClientCallback;

typedef KSafeList<ev_io *> WatcherList;
// Url -> RtmpClient
typedef KSafeMap<string, RtmpClient*> RtmpUrl2ClientMap;

// RtmpClient -> Stream
typedef KSafeList<string> RtmpStreamList;
typedef KSafeMap<RtmpClient*, RtmpStreamList*> RtmpClient2StreamMap;

// Stream -> Url
typedef KSafeMap<string, string> RtmpStream2UrlMap;

class RtmpStreamPool : public RtmpClientCallback {
	friend class RtmpIORunnable;
	friend class RtmpClient;
	friend void recv_rtmp_handler(struct ev_loop *loop, ev_io *w, int revents);

public:
	RtmpStreamPool();
	virtual ~RtmpStreamPool();

public:
	/**
	 * 启动服务
	 * @param maxConnection 最大连接数
	 */
	bool Start(int maxConnection = 1000);

	/**
	 * 停止服务
	 */
	void Stop();

	/**
	 * 是否运行中
	 */
	bool IsRunning();

	/**
	 * 拉流
	 * @param url 链接地址
	 * @param identification 唯一标识
	 * @param errMsg 错误信息
	 * @return 成功失败
	 */
	bool PlayStream(const string& url, const string& identification, string& errMsg);

	/**
	 * 断开流
	 * @param identification 唯一标识
	 * @param errMsg 错误信息
	 * @return 成功失败
	 */
	bool StopStream(const string& identification, string& errMsg);

private:
    void OnConnect(RtmpClient *RtmpClient);
    void OnChangeVideoSpsPps(
    		RtmpClient *RtmpClient,
			const char *sps,
			int sps_size,
			const char *pps,
			int pps_size,
			int naluHeaderSize,
			u_int32_t timestamp);
    void OnRecvVideoFrame(
    		RtmpClient *RtmpClient,
    		const char *data,
			int size,
			u_int32_t timestamp,
			VideoFrameType video_type);
    void OnChangeAudioFormat(
    		RtmpClient *RtmpClient,
			AudioFrameFormat format,
			AudioFrameSoundRate sound_rate,
			AudioFrameSoundSize sound_size,
			AudioFrameSoundType sound_type);
    void OnRecvAudioFrame(
    		RtmpClient *RtmpClient,
    		AudioFrameFormat format,
			AudioFrameSoundRate sound_rate,
			AudioFrameSoundSize sound_size,
			AudioFrameSoundType sound_type,
			char *data,
			int size,
			u_int32_t timestamp);

private:
	void IOHandleThread();
	void IOHandleRecv(ev_io *w, int revents);
	void IOHandleOnDisconnect(ev_io *w);

private:
	void StopEvIO(ev_io *w);

	KMutex mServerMutex;
	bool mRunning;

	int miMaxConnection;

	RtmpIORunnable* mpIORunnable;
	KThread mIOThread;

	ev_loop *mpLoop;
	KMutex mWatcherMutex;
	WatcherList mWatcherList;

	RtmpUrl2ClientMap mRtmpUrl2ClientMap;
	RtmpClient2StreamMap mRtmpClient2StreamMap;
	RtmpStream2UrlMap mRtmpStream2UrlMap;

	RtpRawClient mRtpRawClient;
};
}
#endif /* RTMPDUMP_RTMPSTREAMPOOL_H_ */
