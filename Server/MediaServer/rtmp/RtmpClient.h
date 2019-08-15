//
//  RtmpClient.hpp
//  RtmpClient
//
//  Created by Max on 2017/4/5.
//  Copyright © 2017年 net.qdating. All rights reserved.
//

#ifndef RtmpClient_hpp
#define RtmpClient_hpp

#include <common/LogManager.h>
#include <common/KMutex.h>
#include <common/KThread.h>

#include <media/IDecoder.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
using namespace std;

struct ev_io;
namespace mediaserver {
class RtmpClient;
class RtmpClientCallback {
  public:
    virtual ~RtmpClientCallback(){};
    virtual void OnConnect(RtmpClient *RtmpClient) = 0;
    virtual void OnChangeVideoSpsPps(RtmpClient *RtmpClient, const char *sps, int sps_size, const char *pps, int pps_size, int naluHeaderSize, u_int32_t timestamp) = 0;
    virtual void OnRecvVideoFrame(RtmpClient *RtmpClient, const char *data, int size, u_int32_t timestamp, VideoFrameType video_type) = 0;
    virtual void OnChangeAudioFormat(RtmpClient *RtmpClient,
                                     AudioFrameFormat format,
                                     AudioFrameSoundRate sound_rate,
                                     AudioFrameSoundSize sound_size,
                                     AudioFrameSoundType sound_type) = 0;
    virtual void OnRecvAudioFrame(RtmpClient *RtmpClient,
                                  AudioFrameFormat format,
                                  AudioFrameSoundRate sound_rate,
                                  AudioFrameSoundSize sound_size,
                                  AudioFrameSoundType sound_type,
                                  char *data,
                                  int size,
                                  u_int32_t timestamp) = 0;
};

class ConnectRunnable;
class CheckConnectRunnable;
class RtmpClient {
  public:
    static void GobalInit();

  public:
    RtmpClient();
    ~RtmpClient();

    /**
     设置回调

     @param callback 回调
     */
    void SetCallback(RtmpClientCallback *callback);

    /**
     设置推流视频参数

     @param width 视频宽
     @param height 视频高
     */
    void SetVideoParam(int width, int height);

    /**
     播放流连接

     @param url 连接
     @param recordFilePath flv录制路径
     @param identification 唯一标识
     @return 成功／失败
     */
    bool PlayStream(const string &url, const string &recordFilePath);

    /**
     发布流连接

     @param url 连接
     @return 成功／失败
     */
    bool PublishUrl(const string &url);

    /**
     发送原始h264视频帧

     @param frame 视频帧
     @param frame_size 视频帧大小
     @param timestamp 视频帧时间戳
     @return 成功失败
     */
    bool SendVideoFrame(char *frame, int frame_size, u_int32_t timestamp);
    void AddVideoTimestamp(u_int32_t timestamp);

    /**
     发送原始音频帧

     @param sound_format 音频帧格式
     @param sound_rate 音频帧采样率
     @param sound_size 音频帧精度
     @param sound_type 音频帧声道数
     @param frame 音频帧
     @param frame_size 音频帧大小
     @return 成功失败
     */
    bool SendAudioFrame(AudioFrameFormat sound_format,
                        AudioFrameSoundRate sound_rate,
                        AudioFrameSoundSize sound_size,
                        AudioFrameSoundType sound_type,
                        char *frame,
                        int frame_size,
                        u_int32_t timestamp);
    void AddAudioTimestamp(u_int32_t timestamp);

    /**
     接收数据

     @return 成功／失败
     */
    bool RecvPacket();

    /**
     断开连接
     */
    void Shutdown();

    /**
     关闭连接
     */
    void Close();

  public:
    int GetFD();
    ev_io *GetIO();
    void SetIO(ev_io *w);
    const string& GetUrl();

  public:
    // 接收线程处理
    void ConnectRunnableHandle();
    // 检查超时线程处理
    void CheckConnectRunnableHandle();

  private:
    void Destroy();
    bool Flv2Audio(char *frame, int frame_size, u_int32_t timestamp);
    bool Flv2Video(char *frame, int frame_size, u_int32_t timestamp);
    bool FlvVideo2H264(char *frame, int frame_size, u_int32_t timestamp);
    u_int32_t GetCurrentTime();

    void RecvCmd(char *frame, int frame_size, u_int32_t timestamp);

  private:
    RtmpClientCallback *mpRtmpClientCallback;
    string mUrl;

    // srs_rtmp_t 句柄
    void *mpRtmp;
    // srs_flv_t 句柄
    void *mpFlv;

    // 录制文件
    string mRecordFlvFilePath;

    // 状态锁
    KMutex mClientMutex;
    bool mbRunning;

    // 接收线程
    KThread mConnectThread;
    ConnectRunnable *mpConnectRunnable;

    // 检查连接超时线程
    KThread mCheckConnectThread;
    CheckConnectRunnable *mpCheckConnectRunnable;
    // 连接超时(MS)
    int mConnectTimeout;

    // 收包参数
    char *mpSps;
    int mSpsSize;

    char *mpPps;
    int mPpsSize;

    int mNaluHeaderSize;

    // 发包参数
    u_int32_t mEncodeVideoTimestamp;
    u_int32_t mEncodeAudioTimestamp;
    u_int32_t mSendVideoFrameTimestamp;
    u_int32_t mSendAudioFrameTimestamp;
    
    // 是否播放流
    bool mIsPlay;
    // 是否已经连接
    KMutex mConnectedMutex;
    bool mIsConnected;
    
    // 推流视频参数
    int mWidth;
    int mHeight;

    ev_io *mIO;
};

}
#endif /* RtmpClient_hpp */
