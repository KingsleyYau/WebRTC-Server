/*
 * RtmpStreamPool.cpp
 *
 *  Created on: Jun 13, 2019
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "RtmpStreamPool.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <ev.h>

namespace mediaserver {
class RtmpStreamPool;
/***************************** libev回调函数 **************************************/
void recv_handler(struct ev_loop *loop, ev_io *w, int revents) {
	RtmpStreamPool *pContainer = (RtmpStreamPool *)ev_userdata(loop);
	pContainer->IOHandleRecv(w, revents);
}
/***************************** libev回调函数 **************************************/

class IORunnable : public KRunnable {
public:
	IORunnable(RtmpStreamPool *container) {
		mContainer = container;
	}
	virtual ~IORunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->IOHandleThread();
	}
private:
	RtmpStreamPool *mContainer;
};

RtmpStreamPool::RtmpStreamPool() {
	// TODO Auto-generated constructor stub

	mpIORunnable = new IORunnable(this);
	mpLoop = NULL;

	mRunning = false;
	miMaxConnection = 10;
}

RtmpStreamPool::~RtmpStreamPool() {
	// TODO Auto-generated destructor stub
}

bool RtmpStreamPool::Start(int maxConnection) {
	bool bFlag = true;

	LogAync(
			LOG_MSG,
			"RtmpStreamPool::Start( "
			"[Start] "
			")"
			);

	mServerMutex.lock();
	if( mRunning ) {
		Stop();
	}
	mRunning = true;

	if( bFlag ) {
		/* create watchers */
		for(int i = 0 ; i < maxConnection; i++) {
			::ev_io *w = (::ev_io *)malloc(sizeof(::ev_io));
			if( w != NULL ) {
				mWatcherList.PushBack(w);
			}
		}
		LogAync(
				LOG_STAT,
				"RtmpStreamPool::Start( "
				"[Create watchers OK], "
				"maxConnection : %d "
				")",
				maxConnection
				);
	}

	if( bFlag ) {
		mpLoop = ev_loop_new(EVFLAG_AUTO);//EV_DEFAULT;
	}

	if( bFlag ) {
		// 启动IO监听线程
		if( 0 == mIOThread.Start(mpIORunnable) ) {
			LogAync(
					LOG_ERR_SYS,
					"RtmpStreamPool::Start( "
					"[Create IO thread Fail], "
					"maxConnection : %d "
					")",
					maxConnection
					);
			bFlag = false;
		}
	}

	if( bFlag ) {
		LogAync(
				LOG_MSG,
				"RtmpStreamPool::Start( "
				"[OK], "
				"maxConnection : %d "
				")",
				maxConnection
				);
	} else {
		LogAync(
				LOG_ERR_SYS,
				"RtmpStreamPool::Start( "
				"[Fail], "
				"maxConnection : %d "
				")",
				maxConnection
				);
		Stop();
	}

	mServerMutex.unlock();

	return bFlag;
}

void RtmpStreamPool::Stop() {
	LogAync(
			LOG_MSG,
			"RtmpStreamPool::Stop( "
			"[Start], "
			"maxConnection : %d "
			")",
			miMaxConnection
			);

	mServerMutex.lock();

	if( mRunning ) {
		mRunning = false;

		// 等待IO线程停止
		mIOThread.Stop();

		// 清除监听器队列
		::ev_io* w = NULL;
		while( NULL != ( w = mWatcherList.PopFront() ) ) {
			delete w;
		}

		if( mpLoop ) {
			ev_loop_destroy(mpLoop);
		}
	}

	mServerMutex.unlock();

	LogAync(
			LOG_MSG,
			"RtmpStreamPool::Stop( "
			"[OK], "
			"maxConnection : %d "
			")",
			miMaxConnection
			);

}

bool RtmpStreamPool::IsRunning() {
	return mRunning;
}

bool RtmpStreamPool::PlayStream(const string& url, const string& identification, string& errMsg) {
	LogAync(
			LOG_MSG,
			"RtmpStreamPool::PlayStream( "
			"identification : %s, "
			"url : %s "
			")",
			identification.c_str(),
			url.c_str()
			);

	bool bFlag = true;
	int streamSize = 0;
	bool bNewClient = false;

	RtmpClient *pClient = NULL;
	RtmpStreamList *pRtmpStreamList = NULL;

	mRtmpUrl2ClientMap.Lock();
	mRtmpClient2StreamMap.Lock();
	mRtmpStream2UrlMap.Lock();

	// Find url with identification
	RtmpStream2UrlMap::iterator itrStream2Url = mRtmpStream2UrlMap.Find(identification);
	if( itrStream2Url == mRtmpStream2UrlMap.End() ) {
		mRtmpStream2UrlMap.Insert(identification, url);

		// Find client with url
		RtmpUrl2ClientMap::iterator itr = mRtmpUrl2ClientMap.Find(url);
		if( itr != mRtmpUrl2ClientMap.End() ) {
			// Find client's sub list
			pClient = itr->second;
			RtmpClient2StreamMap::iterator itrStream = mRtmpClient2StreamMap.Find(pClient);
			if( itrStream != mRtmpClient2StreamMap.End() ) {
				pRtmpStreamList = itrStream->second;
			}
		} else {
			// Create new client for new url
			pClient = new RtmpClient();
			pRtmpStreamList = new RtmpStreamList();
			mRtmpUrl2ClientMap.Insert(url, pClient);
			mRtmpClient2StreamMap.Insert(pClient, pRtmpStreamList);

			bNewClient = true;
		}

		// Add identification to client's sub list
		if( pRtmpStreamList ) {
			pRtmpStreamList->PushBack(identification);
			streamSize = pRtmpStreamList->Size();
		}
	} else {
		char tmpBuffer[2046];
		snprintf(tmpBuffer, sizeof(tmpBuffer) - 1, "Stream(%s) for identification(%s) is already exist.", url.c_str(), identification.c_str());
		errMsg = tmpBuffer;

		bFlag = false;
	}

	mRtmpStream2UrlMap.Unlock();
	mRtmpClient2StreamMap.Unlock();
	mRtmpUrl2ClientMap.Unlock();

	if( bNewClient && pClient ) {
		pClient->SetCallback(this);
		pClient->PlayStream(url, "");
	}

	LogAync(
			LOG_WARNING,
			"RtmpStreamPool::PlayStream( "
			"[%s], "
			"identification : %s, "
			"url : %s, "
			"streamSize : %d "
			")",
			bFlag?"Succes":"Fail",
			identification.c_str(),
			url.c_str(),
			streamSize
			);

	return bFlag;
}

bool RtmpStreamPool::StopStream(const string& identification, string& errMsg) {
	LogAync(
			LOG_MSG,
			"RtmpStreamPool::StopStream( "
			"identification : %s "
			")",
			identification.c_str()
			);
	bool bFlag = false;
	int streamSize = 0;
	bool bCloseClient = false;

	string url = "";
	RtmpClient *pClient = NULL;
	RtmpStreamList *pRtmpStreamList = NULL;

	mRtmpUrl2ClientMap.Lock();
	mRtmpClient2StreamMap.Lock();
	mRtmpStream2UrlMap.Lock();

	// Find url with identification
	RtmpStream2UrlMap::iterator itrStream2Url = mRtmpStream2UrlMap.Find(identification);
	if( itrStream2Url != mRtmpStream2UrlMap.End() ) {
		url = itrStream2Url->second;
		bFlag = true;

		// Remove identification for url
		mRtmpStream2UrlMap.Erase(itrStream2Url);

		// Find client with url
		RtmpUrl2ClientMap::iterator itr = mRtmpUrl2ClientMap.Find(url);
		if( itr != mRtmpUrl2ClientMap.End() ) {
			// Find client's sub list
			pClient = itr->second;
			RtmpClient2StreamMap::iterator itrClient2Stream = mRtmpClient2StreamMap.Find(pClient);
			if( itrClient2Stream != mRtmpClient2StreamMap.End() ) {
				pRtmpStreamList = itrClient2Stream->second;

				// Remove identification from client's sub list
				if( pRtmpStreamList ) {
					for(RtmpStreamList::iterator itrStream = pRtmpStreamList->Begin(); itrStream != pRtmpStreamList->End(); pRtmpStreamList++) {
						if(*itrStream == identification) {
							pRtmpStreamList->PopValueUnSafe(itrStream);
							break;
						}
					}

					// Remove client's sub list if no needed
					streamSize = pRtmpStreamList->Size();
					if( streamSize == 0 ) {
						// Remove client's sub list
						delete pRtmpStreamList;
						pRtmpStreamList = NULL;
						mRtmpClient2StreamMap.Erase(itrClient2Stream);

						// Remove url for client
						RtmpUrl2ClientMap::iterator itrUrl2Client = mRtmpUrl2ClientMap.Find(url);
						if( itrUrl2Client != mRtmpUrl2ClientMap.End() ) {
							mRtmpUrl2ClientMap.Erase(itrUrl2Client);
						}

						bCloseClient = true;
					}
				}

			}
		}
	} else {
		char tmpBuffer[2046];
		snprintf(tmpBuffer, sizeof(tmpBuffer) - 1, "Stream for identification(%s) is not found.", identification.c_str());
		errMsg = tmpBuffer;
	}

	mRtmpStream2UrlMap.Unlock();
	mRtmpClient2StreamMap.Unlock();
	mRtmpUrl2ClientMap.Unlock();

	if( bCloseClient && pClient ) {
		pClient->Shutdown();
	}

	LogAync(
			LOG_WARNING,
			"RtmpStreamPool::StopStream( "
			"[%s], "
			"identification : %s, "
			"url : %s, "
			"streamSize : %d "
			")",
			bFlag?"Succes":"Fail",
			identification.c_str(),
			url.c_str(),
			streamSize
			);

	return bFlag;
}

void RtmpStreamPool::IOHandleThread() {
	LogAync(
			LOG_MSG,
			"RtmpStreamPool::IOHandleThread( [Start] )"
			);

	// 增加回调参数
	ev_set_userdata(mpLoop, this);

	while( mRunning ) {
		// 执行epoll_wait
		ev_run(mpLoop, 0);
		usleep(200 * 1000);
	}

	LogAync(
			LOG_MSG,
			"RtmpStreamPool::IOHandleThread( [Exit] )"
			);
}

void RtmpStreamPool::IOHandleRecv(ev_io *w, int revents) {
	RtmpClient *rtmpClient = (RtmpClient *)w->data;

	if( revents & EV_ERROR ) {
		LogAync(
				LOG_WARNING,
				"RtmpStreamPool::IOHandleRecv( "
				"[revents & EV_ERROR], "
				"fd : %d, "
				"rtmpClient : %p "
				")",
				w->fd,
				rtmpClient
				);
		// 停止监听epoll
		StopEvIO(w);

		// 回调断开连接
		IOHandleOnDisconnect(w);

	} else {
		// 读取数据并解析RTMP
		if( !rtmpClient->RecvPacket() ) {
			// 停止监听epoll
			StopEvIO(w);

			// 解析失败, 断开连接
			rtmpClient->Shutdown();

			// 回调断开连接
			IOHandleOnDisconnect(w);
		}
	}
}

void RtmpStreamPool::IOHandleOnDisconnect(ev_io *w) {
	RtmpClient *rtmpClient = (RtmpClient *)w->data;

	LogAync(
			LOG_WARNING,
			"RtmpStreamPool::IOHandleOnDisconnect( "
			"fd : %d, "
			"rtmpClient : %p, "
			"url : %s "
			")",
			w->fd,
			rtmpClient,
			rtmpClient->GetUrl().c_str()
			);

	//
	//	// 回调断开连接
	//	if( mpRtmpStreamPoolCallback != NULL ) {
	//		mpRtmpStreamPoolCallback->OnDisconnect(socket);
	//	}

	rtmpClient->Close();
	delete rtmpClient;

}

void RtmpStreamPool::StopEvIO(ev_io *w) {
	if( w != NULL ) {
		int fd = w->fd;
		LogAync(
				LOG_STAT,
				"RtmpStreamPool::StopEvIO( "
				"fd : %d "
				")",
				fd
				);

		// 停止监听epoll
		mWatcherMutex.lock();
		ev_io_stop(mpLoop, w);
		mWatcherMutex.unlock();

		if( mWatcherList.Size() <= (size_t)miMaxConnection ) {
			// 空闲的缓存小于设定值
			LogAync(
					LOG_STAT,
					"RtmpStreamPool::StopEvIO( "
					"[Return ev_io to idle list], "
					"fd : %d "
					")",
					fd
					);

			mWatcherList.PushBack(w);
		} else {
			// 释放内存
			LogAync(
					LOG_WARNING,
					"RtmpStreamPool::StopEvIO( "
					"[Delete extra ev_io], "
					"fd : %d "
					")",
					fd
					);

			free(w);
		}
	}
}

void RtmpStreamPool::OnConnect(RtmpClient *rtmpClient) {
	LogAync(
			LOG_WARNING,
			"RtmpStreamPool::OnConnect( "
			"rtmpClient : %p, "
			"url : %s "
			")",
			rtmpClient,
			rtmpClient->GetUrl().c_str()
			);

	ev_io *w = NULL;
	int fd = rtmpClient->GetFD();
	if( (w = mWatcherList.PopFront()) != NULL ) {
		// 监听读事件
		LogAync(
				LOG_STAT,
				"RtmpStreamPool::OnConnect( "
				"fd : %d, "
				"rtmpClient : %p, "
				"w : %p "
				")",
				fd,
				rtmpClient,
				w
				);

	} else {
		w = (ev_io *)malloc(sizeof(ev_io));

		LogAync(
				LOG_WARNING,
				"RtmpStreamPool::OnConnect( "
				"[Not enough watcher, new more], "
				"fd : %d, "
				"rtmpClient : %p, "
				"w : %p "
				")",
				fd,
				rtmpClient,
				w
				);
	}


	int iFlag = 1;
	// 设置非阻塞
	int flags = fcntl(fd, F_GETFL, 0);
	flags = flags | O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);

    // 设置ACK马上回复
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &iFlag, sizeof(iFlag));
	// Close（一般不会立即关闭而经历TIME_WAIT的过程）后想继续重用该socket
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &iFlag, sizeof(iFlag));
	// 如果在发送数据的时，希望不经历由系统缓冲区到socket缓冲区的拷贝而影响
	int nZero = 0;
	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &nZero, sizeof(nZero));

	w->data = rtmpClient;
	rtmpClient->SetIO(w);
	ev_io_init(w, recv_handler, fd, EV_READ);

	mWatcherMutex.lock();
	ev_io_start(mpLoop, w);
	mWatcherMutex.unlock();

	mRtpRawClient.Init("192.168.88.138", 9999, 9999);
}

void RtmpStreamPool::OnChangeVideoSpsPps(
		RtmpClient *rtmpClient,
		const char *sps,
		int sps_size,
		const char *pps,
		int pps_size,
		int naluHeaderSize,
		u_int32_t timestamp) {
	LogAync(
			LOG_STAT,
			"RtmpStreamPool::OnChangeVideoSpsPps( "
			"rtmpClient : %p, "
			"timestamp : %u "
			")",
			rtmpClient,
			timestamp
			);
	mRtpRawClient.SetVideoKeyFrameInfoH264(sps, sps_size, pps, pps_size, naluHeaderSize, timestamp);
}

void RtmpStreamPool::OnRecvVideoFrame(
		RtmpClient *rtmpClient,
		const char *data,
		int size,
		u_int32_t timestamp,
		VideoFrameType video_type) {
//	LogAync(
//			LOG_STAT,
//			"RtmpStreamPool::OnRecvVideoFrame( "
//			"rtmpClient : %p, "
//			"timestamp : %u "
//			")",
//			rtmpClient,
//			timestamp
//			);

	mRtpRawClient.SendVideoFrameH264(data, size, timestamp + 900);
}

void RtmpStreamPool::OnChangeAudioFormat(
		RtmpClient *rtmpClient,
		AudioFrameFormat format,
		AudioFrameSoundRate sound_rate,
		AudioFrameSoundSize sound_size,
		AudioFrameSoundType sound_type) {
	LogAync(
			LOG_STAT,
			"RtmpStreamPool::OnChangeAudioFormat( "
			"rtmpClient : %p, "
			"format : %d "
			")",
			rtmpClient,
			format
			);
}

void RtmpStreamPool::OnRecvAudioFrame(
		RtmpClient *rtmpClient,
		AudioFrameFormat format,
		AudioFrameSoundRate sound_rate,
		AudioFrameSoundSize sound_size,
		AudioFrameSoundType sound_type,
		char *data,
		int size,
		u_int32_t timestamp) {
//	LogAync(
//			LOG_STAT,
//			"RtmpStreamPool::OnRecvAudioFrame( "
//			"rtmpClient : %p, "
//			"timestamp : %u "
//			")",
//			rtmpClient,
//			timestamp
//			);
}
}
