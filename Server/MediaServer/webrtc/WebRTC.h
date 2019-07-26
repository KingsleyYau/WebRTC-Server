/*
 * WebRTC.h
 *
 *  Created on: 2019/07/02
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef WEBRTC_WEBRTC_H_
#define WEBRTC_WEBRTC_H_

#include <common/KSafeList.h>

#include <ice/IceClient.h>
#include <rtp/DtlsSession.h>
#include <rtp/RtpSession.h>
#include <socket/ISocketSender.h>
#include <rtp/RtpRawClient.h>


namespace mediaserver {
typedef list<string> RtcpFbList;
typedef struct SdpPayload {
	unsigned int payload_type;
	string encoding_name;
	string encoding_params;
	unsigned int clock_rate;
	string fmtp;
} SdpPayload;

class WebRTC;
class WebRTCCallback {
public:
	virtual ~WebRTCCallback(){};
	virtual void OnWebRTCCreateSdp(WebRTC *rtc, const string& sdp) = 0;
	virtual void OnWebRTCClose(WebRTC *rtc) = 0;
};

class WebRTC : public SocketSender, IceClientCallback {
public:
	WebRTC();
	virtual ~WebRTC();

public:
	static bool GobalInit();

public:
	bool Start(const string& sdp);
	void Stop();

public:
	void SetCallback(WebRTCCallback *callback);

	void SetCustom(void *custom);
	void* GetCustom();

private:
	// SocketSender Implement
	int SendData(const void *data, unsigned int len);
	// IceClientCallback Implement
	void OnIceCandidateGatheringDone(IceClient *ice, const string& type, const string& ip, unsigned int port, const string& ufrag, const string& pwd);
	void OnIceNewSelectedPairFull(IceClient *ice);
	void OnIceRecvData(IceClient *ice, const char *data, unsigned int size, unsigned int streamId, unsigned int componentId);
	void OnIceClose(IceClient *ice);

	/**
	 * 解析SDP
	 */
	bool ParseRemoteSdp(const string& sdp);

private:
	WebRTCCallback *mpWebRTCCallback;

	IceClient mIceClient;
	DtlsSession mDtlsSession;
	RtpSession mRtpSession;

	RtpRawClient mRtpDstAudioClient;
	RtpRawClient mRtpDstVideoClient;

	unsigned int mVideoSSRC;
	unsigned int mAudioSSRC;

	SdpPayload mSdpPayload;
	RtcpFbList mVideoRtcpFbList;

	void *mpCustom;
};

} /* namespace mediaserver */

#endif /* WEBRTC_WEBRTC_H_ */
