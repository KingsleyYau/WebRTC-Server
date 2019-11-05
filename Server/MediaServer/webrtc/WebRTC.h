/*
 * WebRTC.h
 * WebRTC控制器, 管理整个WebRTC流程
 * 1.解析远程SDP
 * 2.开启ICE获取转发端口
 * 3.进行DTLS握手
 * 4.创建本地SDP, 启动FFMEPG接收本地RTP流, 并转发RTMP到Nginx流媒体服务器
 * 5.转发RTP/RTCP到本地流
 *
 *  Created on: 2019/07/02
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef WEBRTC_WEBRTC_H_
#define WEBRTC_WEBRTC_H_

#include <server/MainLoop.h>

#include <common/KSafeList.h>

#include <rtp/DtlsSession.h>
#include <rtp/RtpSession.h>
#include <socket/ISocketSender.h>
#include <rtp/RtpRawClient.h>

#include <webrtc/IceClient.h>

namespace mediaserver {
typedef list<string> RtcpFbList;
typedef struct SdpPayload {
	unsigned int payload_type;
	string encoding_name;
	unsigned int clock_rate;
	string encoding_params;
	string fmtp;
} SdpPayload;

typedef enum WebRTCErrorType {
	WebRTCErrorType_None = 0,
	WebRTCErrorType_Rtp2Rtmp_Start_Fail,
	WebRTCErrorType_Rtp2Rtmp_Exit,
	WebRTCErrorType_Unknow,
} WebRTCErrorType;

const string WebRTCErrorMsg[] = {
	"",
	"WebRTC Rtp Transform Rtmp Start Error.",
	"WebRTC Rtp Transform Rtmp Exit Error.",
	"WebRTC Unknow Error.",
};

typedef enum WebRTCMediaType {
	WebRTCMediaType_BothVideoAudio,
	WebRTCMediaType_OnlyVideo,
	WebRTCMediaType_OnlyAudio,
	WebRTCMediaType_None,
} WebRTCMediaType;

const string WebRTCMediaTypeString[] = {
	"VA",
	"V",
	"A",
	"N",
};

class WebRTC;
class WebRTCCallback {
public:
	virtual ~WebRTCCallback(){};
	virtual void OnWebRTCServerSdp(WebRTC *rtc, const string& sdp, WebRTCMediaType type) = 0;
	virtual void OnWebRTCStartMedia(WebRTC *rtc) = 0;
	virtual void OnWebRTCError(WebRTC *rtc, WebRTCErrorType errType, const string& errMsg) = 0;
	virtual void OnWebRTCClose(WebRTC *rtc) = 0;
};

class WebRTC : public SocketSender, IceClientCallback, MainLoopCallback {
public:
	WebRTC();
	virtual ~WebRTC();

public:
	static bool GobalInit(
			const string& certPath,
			const string& keyPath,
			const string& stunServerIp,
			const string& localIp,
			const string& turnUserName,
			const string& turnPassword
			);

public:
	void SetCallback(WebRTCCallback *callback);
	bool Init(
			const string& rtp2RtmpShellFilePath,
			const string& rtpDstAudioIp = "127.0.0.1",
			unsigned int rtpDstAudioPort = 10000,
			const string& rtpDstVideoIp = "127.0.0.1",
			unsigned int rtpDstVideoPort = 10002
			);
	bool Start(
			const string& sdp,
			const string& rtmpUrl
			);
	void Stop();
	void UpdateCandidate(const string& sdp);
	string GetRtmpUrl();

private:
	// SocketSender Implement
	int SendData(const void *data, unsigned int len);
	// IceClientCallback Implement
	void OnIceCandidateGatheringDone(IceClient *ice, const string& ip, unsigned int port, vector<string> candList, const string& ufrag, const string& pwd);
	void OnIceNewSelectedPairFull(IceClient *ice);
	void OnIceConnected(IceClient *ice);
	void OnIceRecvData(IceClient *ice, const char *data, unsigned int size, unsigned int streamId, unsigned int componentId);
	void OnIceClose(IceClient *ice);
	// MainLoopCallback
	void OnChildExit(int pid);

	/**
	 * 解析远程SDP
	 * @param sdp 远程SDP
	 */
	bool ParseRemoteSdp(const string& sdp);
	/**
	 * 创建本地转发的SDP
	 * @return 本地转发的SDP
	 */
	string CreateLocalSdp();
	/**
	 * 创建本地转发的SDP文件
	 */
	bool CreateLocalSdpFile();
	/**
	 * 删除本地转发的SDP文件
	 */
	void RemoveLocalSdpFile();
	/**
	 * 开始转发RTP到RTMP
	 */
	bool StartRtpTransform();
	/**
	 * 停止转发RTP到RTMP
	 */
	void StopRtpTransform();
	/**
	 * 创建音视频SDP
	 */
	string CreateVideoAudioSdp(const string& candidate, const string& ip, unsigned int port, vector<string> candList, const string& ufrag, const string& pwd);
	/**
	 * 创建视频SDP
	 */
	string CreateVideoOnlySdp(const string& candidate, const string& ip, unsigned int port, vector<string> candList, const string& ufrag, const string& pwd);
	/**
	 * 创建音频SDP
	 */
	string CreateAudioOnlySdp(const string& candidate, const string& ip, unsigned int port, vector<string> candList, const string& ufrag, const string& pwd);

private:
	// Status
	KMutex mClientMutex;
	bool mRunning;

	WebRTCCallback *mpWebRTCCallback;

	IceClient mIceClient;
	DtlsSession mDtlsSession;
	RtpSession mRtpSession;

	RtpRawClient mRtpDstAudioClient;
	RtpRawClient mRtpDstVideoClient;

	string mRtpDstAudioIp;
	unsigned int mRtpDstAudioPort;
	string mRtpDstVideoIp;
	unsigned int mRtpDstVideoPort;

	unsigned int mVideoSSRC;
	unsigned int mAudioSSRC;
	string mVideoMid;
	string mAudioMid;

	SdpPayload mAudioSdpPayload;
	RtcpFbList mAudioRtcpFbList;
	SdpPayload mVideoSdpPayload;
	RtcpFbList mVideoRtcpFbList;

	// 执行转发RTMP的脚本
	string mRtp2RtmpShellFilePath;
	bool mNeedTranscodeVideo;
	// 转发RTMP的链接
	string mRtmpUrl;
	// 转发RTMP脚本的进程ID
	int mRtpTransformPid;
	KMutex mRtpTransformPidMutex;
	// 本地SDP文件
	FILE *mpSdpFile;
	string mSdpFilePath;

	// 需要传输的媒体流类型
	WebRTCMediaType mWebRTCMediaType;
};

} /* namespace mediaserver */

#endif /* WEBRTC_WEBRTC_H_ */
