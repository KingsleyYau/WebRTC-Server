/*
 * WebRTCClient.h
 * WebRTCClient控制器, 管理整个WebRTCClient流程
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

#ifndef WebRTCClient_WebRTCClient_H_
#define WebRTCClient_WebRTCClient_H_

#include <server/MainLoop.h>

#include <common/KSafeList.h>
#include <common/KThread.h>

#include <socket/ISocketSender.h>

#include <rtp/DtlsSession.h>
#include <rtp/RtpSession.h>
#include <rtp/RtpRawClient.h>
#include <webrtc/IceClient.h>

namespace qpidnetwork {
typedef list<string> RtcpFbList;
typedef struct SdpPayload {
	unsigned int payload_type;
	string encoding_name;
	unsigned int clock_rate;
	string encoding_params;
	string fmtp;
} SdpPayload;

typedef enum WebRTCClientErrorType {
	WebRTCClientErrorType_None = 0,
	WebRTCClientErrorType_Rtp2Rtmp_Start_Fail,
	WebRTCClientErrorType_Rtp2Rtmp_Exit,
	WebRTCClientErrorType_Unknow,
} WebRTCClientErrorType;

const string WebRTCClientErrorMsg[] = {
	"",
	"WebRTCClient Rtp Transform Rtmp Start Error.",
	"WebRTCClient Rtp Transform Rtmp Exit Error.",
	"WebRTCClient Unknow Error.",
};

class WebRTCClientRunnable;
class WebRTCClient;
class WebRTCClientCallback {
public:
	virtual ~WebRTCClientCallback(){};
	virtual void OnWebRTCClientServerSdp(WebRTCClient *rtc, const string& sdp) = 0;
	virtual void OnWebRTCClientStartMedia(WebRTCClient *rtc) = 0;
	virtual void OnWebRTCClientError(WebRTCClient *rtc, WebRTCClientErrorType errType, const string& errMsg) = 0;
	virtual void OnWebRTCClientClose(WebRTCClient *rtc) = 0;
};

class WebRTCClient : public SocketSender, IceClientCallback, MainLoopCallback {
	friend class WebRTCClientRunnable;

public:
	WebRTCClient();
	virtual ~WebRTCClient();

public:
	static bool GobalInit(const string& certPath, const string& keyPath, const string& stunServerIp, const string& localIp);

public:
	void SetCallback(WebRTCClientCallback *callback);
	bool Init(
			const string rtp2RtmpShellFilePath,
			const string rtpDstAudioIp = "127.0.0.1",
			unsigned int rtpDstAudioPort = 10000
			);
	bool Start(
			const string sdp,
			const string name,
			bool bTcpFoce = true
			);
	void Stop();
	void Shutdown();
	void UpdateCandidate(const string& sdp);

private:
	// SocketSender Implement
	int SendData(const void *data, unsigned int len);
	// IceClientCallback Implement
	void OnIceCandidateGatheringFail(IceClient *ice, RequestErrorType errType);
	void OnIceCandidateGatheringDone(IceClient *ice, const string& ip, unsigned int port, vector<string> candList, const string& ufrag, const string& pwd);
	void OnIceNewSelectedPairFull(IceClient *ice);
	void OnIceConnected(IceClient *ice);
	void OnIceRecvData(IceClient *ice, const char *data, unsigned int size, unsigned int streamId, unsigned int componentId);
	void OnIceClose(IceClient *ice);
	void OnIceFail(IceClient *ice);

	// MainLoopCallback
	void OnChildExit(int pid);

	/**
	 * 解析远程SDP
	 * @param sdp 远程SDP
	 */
	bool ParseRemoteSdp(const string& sdp);
	/**
	 * 开始转发RTP到RTMP
	 */
	bool StartRtpTransform();
	/**
	 * 停止转发RTP到RTMP
	 */
	void StopRtpTransform();

private:
	void RecvRtpThread();

private:
	// Status
	KMutex mClientMutex;
	bool mRunning;

	WebRTCClientCallback *mpWebRTCClientCallback;

	IceClient mIceClient;
	DtlsSession mDtlsSession;
	RtpSession mRtpSession;

	WebRTCClientRunnable* mpRtpClientRunnable;
	KThread mRtpClientThread;
	RtpRawClient mRtpClient;

	string mRtpDstAudioIp;
	unsigned int mRtpDstAudioPort;

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
	// 转发RTP的链接
	string mRtpUrl;
	// 转发RTMP脚本的进程ID
	int mRtpTransformPid;
	KMutex mRtpTransformPidMutex;
};

} /* namespace qpidnetwork */

#endif /* WebRTCClient_WebRTCClient_H_ */
