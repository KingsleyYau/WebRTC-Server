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
#include <include/ErrCode.h>
#include <include/ForkNotice.h>

// Rtp
#include <rtp/api/rtp_parameters.h>
#include <rtp/DtlsSession.h>
#include <rtp/RtpSession.h>
#include <rtp/RtpRawClient.h>

#include <socket/ISocketSender.h>
#include <webrtc/IceClient.h>

#include <vector>
using namespace std;

namespace mediaserver {
typedef list<string> RtcpFbList;
typedef struct SdpPayload {
	void Reset() {
		payload_type = 0;
		encoding_name = "";
		clock_rate = 0;
		encoding_params = "";
		fmtp = "";
	}

	unsigned int payload_type;
	string encoding_name;
	unsigned int clock_rate;
	string encoding_params;
	string fmtp;
} SdpPayload;

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

class DtlsRunnable;
class RtpRecvRunnable;
class WebRTC;
class WebRTCCallback {
public:
	virtual ~WebRTCCallback(){};
	virtual void OnWebRTCServerSdp(WebRTC *rtc, const string& sdp, WebRTCMediaType type) = 0;
	virtual void OnWebRTCStartMedia(WebRTC *rtc) = 0;
	virtual void OnWebRTCError(WebRTC *rtc, RequestErrorType errType, const string& errMsg) = 0;
	virtual void OnWebRTCClose(WebRTC *rtc) = 0;
};

class WebRTC : public SocketSender, IceClientCallback, MainLoopCallback {
	friend class DtlsRunnable;
	friend class RtpRecvRunnable;

public:
	WebRTC();
	virtual ~WebRTC();

public:
	static bool GobalInit(
			const string& certPath,
			const string& keyPath,
			const string& stunServerIp,
			const string& localIp,
			bool useShareSecret,
			const string& turnUserName,
			const string& turnPassword,
			const string& turnShareSecret
			);

	static void SetDropAudioBeforeVideo(bool bFlag);

public:
	bool Init(
			const string& rtp2RtmpShellFilePath,
			const string& rtmp2RtpShellFilePath,
			const string& rtpDstAudioIp = "127.0.0.1",
			unsigned int rtpDstAudioPort = 10000,
			const string& rtpDstVideoIp = "127.0.0.1",
			unsigned int rtpDstVideoPort = 10002,
			const string& rtpRecvIp = "127.0.0.1",
			unsigned int rtpRecvPort = 10000
			);
	void SetCallback(WebRTCCallback *callback, ForkNotice *forkNotice = NULL);
	bool Start(
			const string& sdp,
			const string& rtmpUrl,
			bool isPull = false,
			bool bControlling = false
			);
	void Stop();
	void UpdateCandidate(const string& sdp);
	string GetRtmpUrl();
	RequestErrorType GetLastErrorType();
	string GetLastErrorMessage();

	/**
	 * 解析远程SDP
	 * @param sdp 远程SDP
	 */
	bool ParseRemoteSdp(const string& sdp);

private:
	// SocketSender Implement
	int SendData(const void *data, unsigned int len);
	// IceClientCallback Implement
	void OnIceCandidateGatheringFail(IceClient *ice, RequestErrorType errType);
	void OnIceCandidateGatheringDone(IceClient *ice, const string& ip, unsigned int port, vector<string> candList, const string& ufrag, const string& pwd);
	void OnIceNewSelectedPairFull(IceClient *ice);
	void OnIceConnected(IceClient *ice);
	void OnIceRecvData(IceClient *ice, const char *data, unsigned int size, unsigned int streamId, unsigned int componentId);
	void OnIceFail(IceClient *ice);
	void OnIceClose(IceClient *ice);

	// MainLoopCallback
	void OnChildExit(int pid);
	// Dtls Handshake Thread Proc
	void DtlsThread();
	// Recv Rtp Thread Proc
	void RecvRtpThread();

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
	/**
	 * 创建视频RTCP-FB
	 */
	string CreateVideoRtcpFb();
	/**
	 * 创建音频RTCP-FB
	 */
	string CreateAudioRtcpFb();
	/**
	 * 创建视频EXTMAP
	 */
	string CreateVideoExtmap();
	/**
	 * 创建音频EXTMAP
	 */
	string CreateAudioExtmap();

private:
	// Status
	KMutex mClientMutex;
	bool mRunning;

	KMutex mParamMutex;
	WebRTCCallback *mpWebRTCCallback;
	ForkNotice *mpForkNotice;

	IceClient mIceClient;
	DtlsSession mDtlsSession;
	RtpSession mRtpSession;

	// DTLS协商线程
	DtlsRunnable* mpDtlsRunnable;
	KThread mDtlsThread;

	// 用于转发SRTP->RTP
	RtpRawClient mRtpDstAudioClient;
	RtpRawClient mRtpDstVideoClient;
	// 用于转发RTP->SRTP
	RtpRawClient mRtpRecvClient;
	// 转发RTP线程
	RtpRecvRunnable* mpRtpRecvRunnable;
	KThread mRtpRecvThread;

	string mRtpDstAudioIp;
	unsigned int mRtpDstAudioPort;
	string mRtpDstVideoIp;
	unsigned int mRtpDstVideoPort;
	string mRtpRecvIp;
	unsigned int mRtpRecvPort;

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
	// 是否转码
	bool mNeedTranscodeVideo;
	// 执行转发RTP的脚本
	string mRtmp2RtpShellFilePath;
	// 转发RTMP的链接
	string mRtmpUrl;
	// 是否拉流
	bool mIsPull;
	// 转发RTMP脚本的进程ID
	int mRtpTransformPid;
	KMutex mRtpTransformPidMutex;
	// 本地SDP文件
	FILE *mpSdpFile;
	string mSdpFilePath;
	string mSdpLogFilePath;

	// 需要传输的媒体流类型
	WebRTCMediaType mWebRTCMediaType;
	bool mWebRTCMediaVideoFirst;

	// ICE打洞
	bool mbCandidate;
	bool mbIceUfrag;
	bool mbIcePwd;

	// 最后一次错误码
	RequestErrorType mLastErrorCode;

	vector<RtpExtension> mVideoExtmap;
	vector<RtpExtension> mAudioExtmap;
};

} /* namespace mediaserver */

#endif /* WEBRTC_WEBRTC_H_ */
