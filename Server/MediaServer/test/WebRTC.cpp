/*
 * WebRTC.cpp
 *
 *  Created on: 2019/07/02
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "WebRTC.h"

// System
#include <signal.h>

#include <include/CommonHeader.h>

// Common
#include <common/LogManager.h>
#include <common/StringHandle.h>
#include <common/CommonFunc.h>

// ThirdParty
#include <libsdp.h>

namespace mediaserver {
class WebRTCRunnable : public KRunnable {
public:
	WebRTCRunnable(WebRTC *container) {
		mContainer = container;
	}
	virtual ~WebRTCRunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->RecvRtpThread();
	}
private:
	WebRTC *mContainer;
};

WebRTC::WebRTC()
:mClientMutex(KMutex::MutexType_Recursive), mRtpTransformPidMutex(KMutex::MutexType_Recursive)
{
	// TODO Auto-generated constructor stub
	mpRtpClientRunnable = new WebRTCRunnable(this);

	mIceClient.SetCallback(this);
	mDtlsSession.SetSocketSender(this);
	mRtpSession.SetRtpSender(this);
	mRtpSession.SetRtcpSender(this);

	mpWebRTCCallback = NULL;

	mVideoSSRC = 0;
	mAudioSSRC = 0;

	mRtpDstAudioPort = 0;

	mRtp2RtmpShellFilePath = "";
	mRtmpUrl = "";

	mRtpTransformPid = 0;

	mRunning = false;
}

WebRTC::~WebRTC() {
	// TODO Auto-generated destructor stub
	if( mpRtpClientRunnable ) {
		delete mpRtpClientRunnable;
		mpRtpClientRunnable = NULL;
	}
}

bool WebRTC::GobalInit(const string& certPath, const string& keyPath, const string& stunServerIp, const string& localIp) {
	bool bFlag = true;

	bFlag &= IceClient::GobalInit(stunServerIp, localIp);
	bFlag &= DtlsSession::GobalInit(certPath, keyPath);
	bFlag &= RtpSession::GobalInit();

	LogAync(
			LOG_ERR,
			"WebRTC::GobalInit( "
			"[%s] "
			")",
			FLAG_2_STRING(bFlag)
			);

	return bFlag;
}

void WebRTC::SetCallback(WebRTCCallback *callback) {
	mpWebRTCCallback = callback;
}

bool WebRTC::Init(
		const string rtp2RtmpShellFilePath,
		const string rtpDstAudioIp,
		unsigned int rtpDstAudioPort
		) {
	bool bFlag = true;
	bFlag &= mRtpClient.Init(rtpDstAudioIp, rtpDstAudioPort, rtpDstAudioPort);

	if ( bFlag ) {
		mRtpDstAudioIp = rtpDstAudioIp;
		mRtpDstAudioPort = rtpDstAudioPort;
		mRtp2RtmpShellFilePath = rtp2RtmpShellFilePath;

		char tmpUrl[2048] = {'\0'};
		sprintf(tmpUrl, "rtp://%s:%u", rtpDstAudioIp.c_str(), rtpDstAudioPort);
		mRtmpUrl = tmpUrl;
	}

	LogAync(
			LOG_DEBUG,
			"WebRTC::Init( "
			"this : %p, "
			"[%s], "
			"rtpDstAudioIp : %s, "
			"rtpDstAudioPort : %u "
			")",
			this,
			FLAG_2_STRING(bFlag),
			rtpDstAudioIp.c_str(),
			rtpDstAudioPort
			);

	return bFlag;
}

bool WebRTC::Start(
		const string& sdp
		) {
	bool bFlag = true;

	mClientMutex.lock();
	if( mRunning ) {
		Stop();
	}
	mRunning = true;

//	bFlag &= ParseRemoteSdp(sdp);
	bFlag &= mIceClient.Start();
	bFlag &= mDtlsSession.Start();

	bFlag &= mRtpClient.Start(NULL, 0, NULL, 0);

	if( bFlag ) {
		// 启动IO监听线程
		if( 0 == mRtpClientThread.Start(mpRtpClientRunnable, "RecvRtpThread") ) {
			LogAync(
					LOG_ALERT,
					"WebRTC::Start( "
					"this : %p, "
					"[Create Rtp Thread Fail] "
					")",
					this
					);
			bFlag = false;
		}
	}

	LogAync(
			LOG_INFO,
			"WebRTC::Start( "
			"this : %p, "
			"[%s] "
			")",
			this,
			FLAG_2_STRING(bFlag)
			);

	if( !bFlag ) {
		Stop();
	}

	mClientMutex.unlock();

	return bFlag;
}

void WebRTC::Stop() {
	LogAync(
			LOG_INFO,
			"WebRTC::Stop( "
			"this : %p "
			")",
			this
			);

	mClientMutex.lock();
	if( mRunning ) {
		// 停止接收
		mRunning = false;

		// 停止媒体流服务
		mIceClient.Stop();
		mDtlsSession.Stop();
		mRtpSession.Stop();
		mRtpClient.Shutdown();

		// 停止转发RTP
		StopRtpTransform();

		mRtpClientThread.Stop();
		mRtpClient.Stop();

		// 还原参数
		mAudioSSRC = 0;
		mVideoSSRC = 0;
	}
	mClientMutex.unlock();

	LogAync(
			LOG_INFO,
			"WebRTC::Stop( "
			"this : %p, "
			"[OK] "
			")",
			this
			);
}

void WebRTC::UpdateCandidate(const string& sdp) {
//	LogAync(
//			LOG_DEBUG,
//			"WebRTC::UpdateCandidate( "
//			"this : %p, "
//			"sdp : %s "
//			")",
//			this,
//			sdp.c_str()
//			);

	mIceClient.SetRemoteSdp(sdp);
}

bool WebRTC::ParseRemoteSdp(const string& sdp) {
	LogAync(
			LOG_INFO,
			"WebRTC::ParseRemoteSdp( "
			"this : %p, "
			"sdp : \n%s"
			")",
			this,
			sdp.c_str()
			);

	bool bFlag = false;

	struct sdp_session *session = NULL;
	int err = -1;
	err = sdp_description_read(sdp.c_str(), &session);
	if( err == 0 ) {
		bFlag = true;
		list_node node;
		sdp_attr *attr = NULL;

		list_walk_entry_forward(&session->attrs, attr, node) {
			if( attr->key && attr->value ) {
				string key(attr->key);
				string value(attr->value);

				if( key == "group" ) {
					LogAync(
							LOG_INFO,
							"WebRTC::ParseRemoteSdp( "
							"this : %p, "
							"[Found Remote Group Bundle], "
							"%s:%s "
							")",
							this,
							key.c_str(),
							value.c_str()
							);

					vector<string> group = StringHandle::splitWithVector(value, " ");
					if( group.size() > 2 ) {
						mAudioMid = group[1];
						mVideoMid = group[2];
					}
				}

			}
		}

		LogAync(
				LOG_DEBUG,
				"WebRTC::ParseRemoteSdp( "
				"this : %p, "
				"media_count : %d "
				")",
				this,
				session->media_count
				);
		sdp_media *media = NULL;
		list_walk_entry_forward(&session->medias, media, node) {
			if ( media ) {
				LogAync(
						LOG_INFO,
						"WebRTC::ParseRemoteSdp( "
						"this : %p, "
						"[Found Remote Media], "
						"media_type : %s, "
						"media_attr_count : %d "
						")",
						this,
						sdp_media_type_str(media->type),
						media->attr_count
						);

				for(int i = 0; i < (int)media->payload_type_array_count; i++) {
					sdp_payload payload = media->payload_type_array[i];
					LogAync(
							LOG_DEBUG,
							"WebRTC::ParseRemoteSdp( "
							"this : %p, "
							"media_type : %s, "
							"payload[%d] : [%d %s/%u/%s %s] "
							")",
							this,
							sdp_media_type_str(media->type),
							i,
							payload.payload_type,
							payload.encoding_name,
							payload.clock_rate,
							payload.encoding_params,
							payload.fmtp
							);

					if ( 0 == strcmp(payload.encoding_name, "H264") ) {
						LogAync(
								LOG_INFO,
								"WebRTC::ParseRemoteSdp( "
								"this : %p, "
								"[Found Remote Media H264 Codec], "
								"media_type : %s, "
								"payload : %d %s/%u/%s, "
								"fmtp : %s "
								")",
								this,
								sdp_media_type_str(media->type),
								payload.payload_type,
								payload.encoding_name,
								payload.clock_rate,
								payload.encoding_params,
								payload.fmtp
								);

						mVideoSdpPayload.payload_type = payload.payload_type;
						mVideoSdpPayload.encoding_name = payload.encoding_name?payload.encoding_name:"";
						mVideoSdpPayload.clock_rate = payload.clock_rate;
						mVideoSdpPayload.encoding_params = payload.encoding_params?payload.encoding_params:"";
						mVideoSdpPayload.fmtp = payload.fmtp?payload.fmtp:"";

						break;

					} else if ( 0 == strcmp(payload.encoding_name, "opus") ) {
						LogAync(
								LOG_INFO,
								"WebRTC::ParseRemoteSdp( "
								"this : %p, "
								"[Found Remote Media OPUS Codec], "
								"media_type : %s, "
								"payload : %d %s/%u/%s, "
								"fmtp : %s "
								")",
								this,
								sdp_media_type_str(media->type),
								payload.payload_type,
								payload.encoding_name,
								payload.clock_rate,
								payload.encoding_params,
								payload.fmtp
								);

						mAudioSdpPayload.payload_type = payload.payload_type;
						mAudioSdpPayload.encoding_name = payload.encoding_name?payload.encoding_name:"";
						mAudioSdpPayload.clock_rate = payload.clock_rate;
						mAudioSdpPayload.encoding_params = payload.encoding_params?payload.encoding_params:"";
						mAudioSdpPayload.fmtp = payload.fmtp?payload.fmtp:"";
						break;
					}
				}

				list_walk_entry_forward(&media->attrs, attr, node) {
					if( attr->key && attr->value ) {
						string key(attr->key);
						string value(attr->value);

						LogAync(
								LOG_DEBUG,
								"WebRTC::ParseRemoteSdp( "
								"this : %p, "
								"media_type : %s, "
								"attr : [%s %s] "
								")",
								this,
								sdp_media_type_str(media->type),
								key.c_str(),
								value.c_str()
								);

						if ( key == "ssrc" ) {
							string::size_type pos = value.find(" ", 0);
							if( pos != string::npos ) {
								string ssrc = value.substr(0, pos);
								if ( media->type == SDP_MEDIA_TYPE_AUDIO ) {
									if ( mAudioSSRC == 0 ) {
										mAudioSSRC = atoll(ssrc.c_str());
										break;
									}
								} else if ( media->type == SDP_MEDIA_TYPE_VIDEO ) {
									if ( mVideoSSRC == 0 ) {
										mVideoSSRC = atoll(ssrc.c_str());
										break;
									}
								}
							}
						} else if( key == "rtcp-fb" ) {
							string::size_type pos = value.find(" ", 0);
							if( pos != string::npos ) {
								string payload = value.substr(0, pos);

								if ( media->type == SDP_MEDIA_TYPE_AUDIO ) {
//									if( mAudioSdpPayload.payload_type == atoi(payload.c_str()) ) {
//										string rtcpFb = key + ":" + value;
//
//										LogAync(
//												LOG_INFO,
//												"WebRTC::ParseRemoteSdp( "
//												"this : %p, "
//												"[Found Remote Audio RTCP Feedback], "
//												"media_type : %s, "
//												"%s "
//												")",
//												this,
//												sdp_media_type_str(media->type),
//												rtcpFb.c_str()
//												);
//
//										mAudioRtcpFbList.push_back(rtcpFb);
//									}
								} else if ( media->type == SDP_MEDIA_TYPE_VIDEO ) {
									if( (int)mVideoSdpPayload.payload_type == atoi(payload.c_str()) ) {
										string rtcpFb = key + ":" + value;

										LogAync(
												LOG_INFO,
												"WebRTC::ParseRemoteSdp( "
												"this : %p, "
												"[Found Remote Video RTCP Feedback], "
												"media_type : %s, "
												"%s "
												")",
												this,
												sdp_media_type_str(media->type),
												rtcpFb.c_str()
												);

										mVideoRtcpFbList.push_back(rtcpFb);
									}
								}
							}
						}
					}
				}
			}
		}

		LogAync(
				LOG_INFO,
				"WebRTC::ParseRemoteSdp( "
				"this : %p, "
				"[Parse Remote SDP OK], "
				"mVideoSdpPayload : %d %s/%u, "
				"fmtp : %s, "
				"mAudioSSRC : %u, "
				"mAudioMid : %s, "
				"mVideoSSRC : %u, "
				"mVideoMid : %s "
				")",
				this,
				mVideoSdpPayload.payload_type,
				mVideoSdpPayload.encoding_name.c_str(),
				mVideoSdpPayload.clock_rate,
				mVideoSdpPayload.fmtp.c_str(),
				mAudioSSRC,
				mAudioMid.c_str(),
				mVideoSSRC,
				mVideoMid.c_str()
				);
	}

	if( bFlag ) {
		mIceClient.SetRemoteSdp(sdp);
	}

	return bFlag;
}

bool WebRTC::StartRtpTransform() {
	bool bFlag = true;

	if ( bFlag ) {
		pid_t pid = fork();
		if ( pid < 0 ) {
			LogAync(
					LOG_ALERT,
					"WebRTC::StartRtpTransform( "
					"this : %p, "
					"[Can't Fork New Process Error] "
					")",
					this
					);
			bFlag = false;
		} else if ( pid > 0 ) {
			LogAync(
					LOG_INFO,
					"WebRTC::StartRtpTransform( "
					"this : %p, "
					"[Fork New Process OK], "
					"pid : %u "
					")",
					this,
					pid
					);
			mRtpTransformPid = pid;
			MainLoop::GetMainLoop()->StartWatchChild(mRtpTransformPid, this);
		} else {
			int ret = execle("/bin/sh", "sh", mRtp2RtmpShellFilePath.c_str(), mRtmpUrl.c_str(), NULL, NULL);
			exit(EXIT_SUCCESS);
		}
	}

	return bFlag;
}

void WebRTC::StopRtpTransform() {
	LogAync(
			LOG_INFO,
			"WebRTC::StopRtpTransform( "
			"this : %p, "
			"pid : %d "
			")",
			this,
			mRtpTransformPid
			);

	// 不需要锁, 内部有锁, 找不到就放过
	MainLoop::GetMainLoop()->StopWatchChild(mRtpTransformPid);

	mRtpTransformPidMutex.lock();
	if ( mRtpTransformPid != 0 ) {
		LogAync(
				LOG_DEBUG,
				"WebRTC::StopRtpTransform( "
				"this : %p, "
				"pid : %d "
				")",
				this,
				mRtpTransformPid
				);
		kill(mRtpTransformPid, SIGTERM);
		mRtpTransformPid = 0;
	}
	mRtpTransformPidMutex.unlock();

	LogAync(
			LOG_INFO,
			"WebRTC::StopRtpTransform( "
			"this : %p, "
			"[OK], "
			"pid : %d "
			")",
			this,
			mRtpTransformPid
			);
}

int WebRTC::SendData(const void *data, unsigned int len) {
	// Send RTP data through ICE data channel
	return mIceClient.SendData(data, len);
}

void WebRTC::OnIceCandidateGatheringDone(IceClient *ice, const string& ip, unsigned int port, vector<string> candList, const string& ufrag, const string& pwd) {
	string candidate;
	for(int i = 0; i < (int)candList.size(); i++) {
		candidate += candList[i];
	}

	LogAync(
			LOG_INFO,
			"WebRTC::OnIceCandidateGatheringDone( "
			"this : %p, "
			"ice : %p, "
			"ip : %s, "
			"port : %u, "
			"ufrag : %s, "
			"pwd : %s, "
			"candidate : \n%s"
			")",
			this,
			ice,
			ip.c_str(),
			port,
			ufrag.c_str(),
			pwd.c_str(),
			candidate.c_str()
			);

	char sdp[4096] = {'0'};
	snprintf(sdp, sizeof(sdp) - 1,
			"v=0\n"
			"o=- 8792925737725123967 2 IN IP4 127.0.0.1\n"
			"s=-\n"
			"t=0 0\n"
			"a=group:BUNDLE 0 1\n"
			"a=msid-semantic: WMS\n"
			"m=audio %u UDP/TLS/RTP/SAVPF 111\n"
			"c=IN IP4 %s\n"
			"a=rtcp:9 IN IP4 0.0.0.0\n"
			"%s"
			"a=ice-ufrag:%s\n"
			"a=ice-pwd:%s\n"
			"a=ice-options:trickle\n"
			"a=fingerprint:sha-256 %s\n"
			"a=setup:actpass\n"
			"a=mid:0\n"
			"a=sendonly\n"
			"a=rtcp-mux\n"
			"a=rtpmap:111 opus/48000/2\n"
			"a=fmtp:111 minptime=10;useinbandfec=1\n"
			"a=ssrc:305419897 cname:audio\n"
			"m=video 9 UDP/TLS/RTP/SAVPF 102\n"
			"c=IN IP4 0.0.0.0\n"
			"a=rtcp:9 IN IP4 0.0.0.0\n"
			"a=ice-ufrag:%s\n"
			"a=ice-pwd:%s\n"
			"a=ice-options:trickle\n"
			"a=fingerprint:sha-256 %s\n"
			"a=setup:actpass\n"
			"a=mid:1\n"
			"a=sendonly\n"
			"a=rtcp-mux\n"
			"a=rtcp-rsize\n"
			"a=rtpmap:102 H264/90000\n"
			"a=fmtp:102 packetization-mode=1;profile-level-id=42e01f\n"
			"a=ssrc:305419896 cname:video\n",
			port,
			ip.c_str(),
			candidate.c_str(),
			ufrag.c_str(),
			pwd.c_str(),
			DtlsSession::GetFingerprint(),
			ufrag.c_str(),
			pwd.c_str(),
			DtlsSession::GetFingerprint()
			);

	string sdpStr = sdp;

	if( mpWebRTCCallback ) {
		mpWebRTCCallback->OnWebRTCServerSdp(this, sdpStr);
	}

}

void WebRTC::OnIceNewSelectedPairFull(IceClient *ice) {
	LogAync(
			LOG_INFO,
			"WebRTC::OnIceNewSelectedPairFull( "
			"this : %p, "
			"ice : %p, "
			"local : %s, "
			"remote : %s "
			")",
			this,
			ice,
			ice->GetLocalAddress().c_str(),
			ice->GetRemoteAddress().c_str()
			);
}

void WebRTC::OnIceConnected(IceClient *ice) {
	LogAync(
			LOG_INFO,
			"WebRTC::OnIceConnected( "
			"this : %p, "
			"ice : %p "
			")",
			this,
			ice
			);
	mDtlsSession.Handshake();
}

void WebRTC::OnIceRecvData(IceClient *ice, const char *data, unsigned int size, unsigned int streamId, unsigned int componentId) {
//	LogAync(
//			LOG_DEBUG,
//			"WebRTC::OnIceRecvData( "
//			"this : %p, "
//			"ice : %p, "
//			"streamId : %u, "
//			"componentId : %u, "
//			"size : %d, "
//			"data[0] : 0x%X "
//			")",
//			this,
//			ice,
//			streamId,
//			componentId,
//			size,
//			(unsigned char)data[0]
//			);

	bool bFlag = false;

	char pkt[RTP_MAX_LEN] = {0};
	unsigned int pktSize = size;

	if ( size > RTP_MAX_LEN ) {
		LogAync(
				LOG_WARNING,
				"WebRTC::OnIceRecvData( "
				"this : %p, "
				"[Unknow Data Format], "
				"ice : %p, "
				"streamId : %u, "
				"componentId : %u, "
				"size : %d, "
				"data[0] : 0x%X "
				")",
				this,
				ice,
				streamId,
				componentId,
				size,
				(unsigned char)data[0]
				);
		return;
	}

	if( DtlsSession::IsDTLS(data, size) ) {
		bFlag = mDtlsSession.RecvFrame(data, size);
		if( bFlag ) {
			// Check Handshake status
			DtlsSessionStatus status = mDtlsSession.GetDtlsSessionStatus();
			if( status == DtlsSessionStatus_HandshakeDone ) {
				LogAync(
						LOG_INFO,
						"WebRTC::OnIceRecvData( "
						"this : %p, "
						"[DTLS Handshake OK] "
						")",
						this
						);

				bool bStart = false;
				if( StartRtpTransform() ) {
					char localKey[SRTP_MASTER_LENGTH];
					int localSize = 0;
					mDtlsSession.GetServerKey(localKey, localSize);
					char remoteKey[SRTP_MASTER_LENGTH];
					int remoteSize = 0;
					mDtlsSession.GetClientKey(remoteKey, remoteSize);

					bStart = mRtpSession.Start(localKey, localSize, remoteKey, remoteSize);
				}

				if ( bStart ) {
					if( mpWebRTCCallback ) {
						mpWebRTCCallback->OnWebRTCStartMedia(this);
					}
				} else {
					if( mpWebRTCCallback ) {
						mpWebRTCCallback->OnWebRTCError(this, WebRTCErrorType_Rtp2Rtmp_Start_Fail, WebRTCErrorMsg[WebRTCErrorType_Rtp2Rtmp_Start_Fail]);
					}
				}

			} else if ( status == DtlsSessionStatus_Alert ) {
				LogAync(
						LOG_WARNING,
						"WebRTC::OnIceRecvData( "
						"this : %p, "
						"[DTLS Alert] "
						")",
						this
						);

				if( mpWebRTCCallback ) {
					mpWebRTCCallback->OnWebRTCClose(this);
				}
			}
		}
	} else if( RtpSession::IsRtp(data, size) ) {
		bFlag = mRtpSession.RecvRtpPacket(data, size, pkt, pktSize);
//		if( bFlag ) {
//			unsigned int ssrc = RtpSession::GetRtpSSRC(pkt, pktSize);
//			if( ssrc == mAudioSSRC ) {
//				mRtpDstAudioClient.SendRtpPacket(pkt, pktSize);
//			} else if ( ssrc == mVideoSSRC ) {
//				mRtpDstVideoClient.SendRtpPacket(pkt, pktSize);
//			}
//		}
	} else if( RtpSession::IsRtcp(data, size) ){
		bFlag = mRtpSession.RecvRtcpPacket(data, size, pkt, pktSize);
//		if( bFlag ) {
//			unsigned int ssrc = RtpSession::GetRtcpSSRC(pkt, pktSize);
//			if( ssrc == mAudioSSRC ) {
//				mRtpDstAudioClient.SendRtcpPacket(pkt, pktSize);
//			} else if ( ssrc == mVideoSSRC ) {
//				mRtpDstVideoClient.SendRtcpPacket(pkt, pktSize);
//			}
//		}
	} else {
		LogAync(
				LOG_WARNING,
				"WebRTC::OnIceRecvData( "
				"this : %p, "
				"[Unknow Data Format], "
				"ice : %p, "
				"streamId : %u, "
				"componentId : %u, "
				"size : %d, "
				"data[0] : 0x%X "
				")",
				this,
				ice,
				streamId,
				componentId,
				size,
				(unsigned char)data[0]
				);
	}
}

void WebRTC::OnIceClose(IceClient *ice) {
	LogAync(
			LOG_WARNING,
			"WebRTC::OnIceClose( "
			"this : %p, "
			"ice : %p, "
			"local : %s, "
			"remote : %s "
			")",
			this,
			ice,
			ice->GetLocalAddress().c_str(),
			ice->GetRemoteAddress().c_str()
			);

	if( mpWebRTCCallback ) {
		mpWebRTCCallback->OnWebRTCClose(this);
	}
}

void WebRTC::OnChildExit(int pid) {
	LogAync(
			LOG_INFO,
			"WebRTC::OnChildExit( "
			"this : %p, "
			"pid : %d "
			")",
			this,
			pid
			);
	mRtpTransformPidMutex.lock();
	mRtpTransformPid = 0;
	mRtpTransformPidMutex.unlock();

	if( mpWebRTCCallback ) {
		mpWebRTCCallback->OnWebRTCError(this, WebRTCErrorType_Rtp2Rtmp_Exit, WebRTCErrorMsg[WebRTCErrorType_Rtp2Rtmp_Exit]);
	}
}

void WebRTC::RecvRtpThread() {
	LogAync(
			LOG_INFO,
			"WebRTC::RecvRtpThread( [Start] )"
			);

	while ( mRunning ) {
		char pkt[2048] = {0};
		unsigned int pktSize = sizeof(pkt);
		if (mRtpClient.RecvRtpPacket(pkt, pktSize) ) {
			mRtpSession.SendRtpPacket(pkt, pktSize);
		} else {
			break;
		}

		usleep(10);
	}

	LogAync(
			LOG_INFO,
			"WebRTC::RecvRtpThread( [Exit] )"
			);
}
} /* namespace mediaserver */
