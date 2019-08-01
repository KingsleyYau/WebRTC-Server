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

// Common
#include <common/LogManager.h>
#include <common/StringHandle.h>
#include <common/CommonFunc.h>

// ThirdParty
#include <libsdp.h>

namespace mediaserver {

WebRTC::WebRTC()
:mRtpTransformPidMutex(KMutex::MutexType_Recursive) {
	// TODO Auto-generated constructor stub
	mIceClient.SetCallback(this);
	mDtlsSession.SetSocketSender(this);
	mRtpSession.SetRtpSender(this);
	mRtpSession.SetRtcpSender(this);

	mpWebRTCCallback = NULL;

	mVideoSSRC = 0;
	mAudioSSRC = 0;

	mRtpDstAudioPort = 0;
	mRtpDstVideoPort = 0;

	mRtp2RtmpShellFilePath = "";
	mRtmpUrl = "";

	mpSdpFile = NULL;
	mSdpFilePath = "";
	mRtpTransformPid = 0;
}

WebRTC::~WebRTC() {
	// TODO Auto-generated destructor stub
}

bool WebRTC::GobalInit() {
	bool bFlag = true;

	bFlag &= IceClient::GobalInit();
	bFlag &= DtlsSession::GobalInit();
	bFlag &= RtpSession::GobalInit();

	LogAync(
			LOG_ERR_USER,
			"WebRTC::GobalInit( "
			"[%s] "
			")",
			bFlag?"OK":"Fail"
			);

	return bFlag;
}

void WebRTC::SetCallback(WebRTCCallback *callback) {
	mpWebRTCCallback = callback;
}

bool WebRTC::Init(
		const string& rtp2RtmpShellFilePath,
		const string& rtpDstAudioIp,
		unsigned int rtpDstAudioPort,
		const string& rtpDstVideoIp,
		unsigned int rtpDstVideoPort
		) {
	bool bFlag = true;
	bFlag &= mRtpDstAudioClient.Init(rtpDstAudioIp, rtpDstAudioPort, rtpDstAudioPort);
	bFlag &= mRtpDstVideoClient.Init(rtpDstVideoIp, rtpDstVideoPort, rtpDstVideoPort);

	if ( bFlag ) {
		mRtpDstAudioIp = rtpDstAudioIp;
		mRtpDstAudioPort = rtpDstAudioPort;
		mRtpDstVideoIp = rtpDstVideoIp;
		mRtpDstVideoPort = rtpDstVideoPort;
		mRtp2RtmpShellFilePath = rtp2RtmpShellFilePath;
	}

	LogAync(
			LOG_STAT,
			"WebRTC::Init( "
			"this : %p, "
			"[%s], "
			"rtpDstAudioIp : %s, "
			"rtpDstAudioPort : %u, "
			"rtpDstVideoIp : %s, "
			"rtpDstVideoPort : %u "
			")",
			this,
			bFlag?"OK":"Fail",
			rtpDstAudioIp.c_str(),
			rtpDstAudioPort,
			rtpDstVideoIp.c_str(),
			rtpDstVideoPort
			);

	return bFlag;
}

bool WebRTC::Start(
		const string& sdp,
		const string& rtmpUrl
		) {
	bool bFlag = true;

	bFlag &= ParseRemoteSdp(sdp);
	bFlag &= mIceClient.Start();
	bFlag &= mDtlsSession.Start();

	bFlag &= mRtpDstAudioClient.Start(NULL, 0, NULL, 0);
	bFlag &= mRtpDstVideoClient.Start(NULL, 0, NULL, 0);

	string tmpDir = "/tmp/webrtc";
	MakeDir(tmpDir);
	char sdpFilePathTmp[MAX_PATH] = {'0'};
	snprintf(sdpFilePathTmp, sizeof(sdpFilePathTmp) - 1, "%s/%d_%d.sdp", tmpDir.c_str(), mRtpDstAudioPort, mRtpDstVideoPort);
	mSdpFilePath = sdpFilePathTmp;

	mRtmpUrl = rtmpUrl;

	LogAync(
			LOG_WARNING,
			"WebRTC::Start( "
			"this : %p, "
			"[%s] "
			")",
			this,
			bFlag?"OK":"Fail"
			);

	if( !bFlag ) {
		Stop();
	}

	return bFlag;
}

void WebRTC::Stop() {
	LogAync(
			LOG_WARNING,
			"WebRTC::Stop( "
			"this : %p "
			")",
			this
			);

	// 停止媒体流服务
	mIceClient.Stop();
	mDtlsSession.Stop();
	mRtpSession.Stop();
	mRtpDstAudioClient.Stop();
	mRtpDstVideoClient.Stop();

	// 停止转发RTMP
	StopRtpTransform();

	// 还原参数
	mAudioSSRC = 0;
	mVideoSSRC = 0;

	LogAync(
			LOG_WARNING,
			"WebRTC::Stop( "
			"this : %p, "
			"[OK] "
			")",
			this
			);
}

void WebRTC::UpdateCandidate(const string& sdp) {
//	LogAync(
//			LOG_STAT,
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
			LOG_MSG,
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
							LOG_MSG,
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
				LOG_STAT,
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
						LOG_MSG,
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

				for(int i = 0; i < media->payload_type_array_count; i++) {
					sdp_payload payload = media->payload_type_array[i];
					LogAync(
							LOG_STAT,
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
								LOG_MSG,
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
								LOG_MSG,
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
								LOG_STAT,
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

								if( mVideoSdpPayload.payload_type == atoi(payload.c_str()) ) {
									string rtcpFb = key + ":" + value;

									LogAync(
											LOG_MSG,
											"WebRTC::ParseRemoteSdp( "
											"this : %p, "
											"[Found Remote Media RTCP Feedback], "
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

		LogAync(
				LOG_MSG,
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

string WebRTC::CreateLocalSdp() {
	/**
	 * 这里必须把视频放前面, 否则FFMPEG会解析不了视频
	 */
	char sdp[4096] = {'0'};
	snprintf(sdp, sizeof(sdp) - 1,
			"SDP:"
			"v=0\n"
			"o=- 0 0 IN IP4 127.0.0.1\n"
			"s=No Name\n"
			"t=0 0\n"
			"m=video %u RTP/AVP %u\n"
			"c=IN IP4 %s\n"
			"a=rtpmap:%u %s/%u\n"
			"a=fmtp:%u %s\n"
			"m=audio %u RTP/AVP %u\n"
			"c=IN IP4 %s\n"
			"a=rtpmap:%u %s/%u%s\n"
			"a=fmtp:%u %s\n",
			mRtpDstVideoPort,
			mVideoSdpPayload.payload_type,
			mRtpDstVideoIp.c_str(),
			mVideoSdpPayload.payload_type,
			mVideoSdpPayload.encoding_name.c_str(),
			mVideoSdpPayload.clock_rate,
			mVideoSdpPayload.payload_type,
			mVideoSdpPayload.fmtp.c_str(),
			mRtpDstAudioPort,
			mAudioSdpPayload.payload_type,
			mRtpDstAudioIp.c_str(),
			mAudioSdpPayload.payload_type,
			mAudioSdpPayload.encoding_name.c_str(),
			mAudioSdpPayload.clock_rate,
			(mAudioSdpPayload.encoding_params.length() > 0)?("/" + mAudioSdpPayload.encoding_params).c_str():"",
			mAudioSdpPayload.payload_type,
			mAudioSdpPayload.fmtp.c_str()
			);

	LogAync(
			LOG_WARNING,
			"WebRTC::CreateLocalSdp( "
			"this : %p, "
			"sdp :\n%s"
			")",
			this,
			sdp
			);

	return string(sdp);
}

bool WebRTC::CreateLocalSdpFile() {
	bool bFlag = false;
	mpSdpFile = fopen(mSdpFilePath.c_str(), "w+b");
	if ( mpSdpFile ) {
		string sdp = CreateLocalSdp();
		size_t size = fwrite(sdp.c_str(), sizeof(char), sdp.length(), mpSdpFile);

		if ( size == sdp.length() ) {
			bFlag = true;
			fflush(mpSdpFile);
		}

        fclose(mpSdpFile);
        mpSdpFile = NULL;
	}

	if ( !bFlag ) {
		LogAync(
				LOG_ERR_USER,
				"WebRTC::CreateLocalSdp( "
				"this : %p, "
				"[Fail], "
				"mSdpFilePath : %s "
				")",
				this,
				mSdpFilePath.c_str()
				);
	}

	return bFlag;
}

void WebRTC::RemoveLocalSdpFile() {
    if (mpSdpFile) {
        fclose(mpSdpFile);
        mpSdpFile = NULL;
    }
    remove(mSdpFilePath.c_str());
}

bool WebRTC::StartRtpTransform() {
	bool bFlag = CreateLocalSdpFile();

	if ( bFlag ) {
		pid_t pid = fork();
		if ( pid < 0 ) {
			LogAync(
					LOG_ERR_SYS,
					"WebRTC::StartRtpTransform( "
					"this : %p, "
					"[Can't Fork New Process Error] "
					")",
					this
					);
			bFlag = false;
		} else if ( pid > 0 ) {
			LogAync(
					LOG_MSG,
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
			int ret = execle("/bin/sh", "sh", mRtp2RtmpShellFilePath.c_str(), mSdpFilePath.c_str(), mRtmpUrl.c_str(), NULL, NULL);
			exit(EXIT_SUCCESS);
		}
	}

	return bFlag;
}

void WebRTC::StopRtpTransform() {
	// 不需要锁, 内部有锁, 找不到就放过
	MainLoop::GetMainLoop()->StopWatchChild(mRtpTransformPid);

	mRtpTransformPidMutex.lock();
	if ( mRtpTransformPid != 0 ) {
		LogAync(
				LOG_ERR_SYS,
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

	RemoveLocalSdpFile();
}

int WebRTC::SendData(const void *data, unsigned int len) {
	// Send RTP data through ICE data channel
	return mIceClient.SendData(data, len);
}

void WebRTC::OnIceCandidateGatheringDone(IceClient *ice, const string& type, const string& ip, unsigned int port, const string& ufrag, const string& pwd) {
	LogAync(
			LOG_WARNING,
			"WebRTC::OnIceCandidateGatheringDone( "
			"this : %p, "
			"ice : %p, "
			"type : %s, "
			"ip : %s, "
			"port : %u, "
			"ufrag : %s, "
			"pwd : %s "
			")",
			this,
			ice,
			type.c_str(),
			ip.c_str(),
			port,
			ufrag.c_str(),
			pwd.c_str()
			);

	char sdp[4096] = {'0'};
	snprintf(sdp, sizeof(sdp) - 1,
			"v=0\n"
			"o=- 8792925737725123967 2 IN IP4 127.0.0.1\n"
			"s=-\n"
			"t=0 0\n"
			"a=group:BUNDLE %s %s\n"
			"a=msid-semantic: WMS\n"
			"m=audio %u UDP/TLS/RTP/SAVPF %u\n"
			"c=IN IP4 %s\n"
			"a=rtcp:9 IN IP4 0.0.0.0\n"
			"a=ice-ufrag:%s\n"
			"a=ice-pwd:%s\n"
			"a=candidate:4 1 UDP 335544831 192.168.88.133 %u typ %s raddr %s rport 9\n"
			"a=ice-options:trickle\n"
			"a=fingerprint:sha-256 %s\n"
			"a=setup:active\n"
			"a=mid:%s\n"
			"a=recvonly\n"
			"a=rtcp-mux\n"
			"a=rtpmap:%u %s/%u%s\n"
			"a=rtcp-fb:%u transport-cc\n"
			"a=fmtp:%u minptime=10;useinbandfec=1\n"
			"m=video 9 UDP/TLS/RTP/SAVPF %u\n"
			"c=IN IP4 %s\n"
			"a=rtcp:9 IN IP4 0.0.0.0\n"
			"a=ice-ufrag:%s\n"
			"a=ice-pwd:%s\n"
			"a=ice-options:trickle\n"
			"a=fingerprint:sha-256 %s\n"
			"a=setup:active\n"
			"a=mid:%s\n"
			"a=recvonly\n"
			"a=rtcp-mux\n"
			"a=rtcp-rsize\n"
			"a=rtpmap:%u %s/%u\n",
			mAudioMid.c_str(),
			mVideoMid.c_str(),
			port,
			mAudioSdpPayload.payload_type,
			ip.c_str(),
			ufrag.c_str(),
			pwd.c_str(),
			port,
			type.c_str(),
			ip.c_str(),
			DtlsSession::GetFingerprint(),
			mAudioMid.c_str(),
			mAudioSdpPayload.payload_type,
			mAudioSdpPayload.encoding_name.c_str(),
			mAudioSdpPayload.clock_rate,
			(mAudioSdpPayload.encoding_params.length() > 0)?("/" + mAudioSdpPayload.encoding_params).c_str():"",
			mAudioSdpPayload.payload_type,
			mAudioSdpPayload.payload_type,
			mVideoSdpPayload.payload_type,
			ip.c_str(),
			ufrag.c_str(),
			pwd.c_str(),
			DtlsSession::GetFingerprint(),
			mVideoMid.c_str(),
			mVideoSdpPayload.payload_type,
			mVideoSdpPayload.encoding_name.c_str(),
			mVideoSdpPayload.clock_rate
			);

	string sdpStr = sdp;
	string rtcpFb = "";
	while( !mVideoRtcpFbList.empty() ) {
		rtcpFb = mVideoRtcpFbList.front();
		mVideoRtcpFbList.pop_front();

		if( rtcpFb.length() > 0 ) {
			sdpStr += "a=";
			sdpStr += rtcpFb;
			sdpStr += "\n";
		}
	}

	char fmtp[256] = {'0'};
	snprintf(fmtp, sizeof(fmtp) - 1,
			"a=fmtp:%u %s\n",
			mVideoSdpPayload.payload_type,
			mVideoSdpPayload.fmtp.c_str()
			);
	sdpStr += fmtp;

	if( mpWebRTCCallback ) {
		mpWebRTCCallback->OnWebRTCServerSdp(this, sdpStr);
	}

}

void WebRTC::OnIceNewSelectedPairFull(IceClient *ice) {
	LogAync(
			LOG_WARNING,
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
	mDtlsSession.Handshake();
}

void WebRTC::OnIceRecvData(IceClient *ice, const char *data, unsigned int size, unsigned int streamId, unsigned int componentId) {
	LogAync(
			LOG_STAT,
			"WebRTC::OnIceRecvData( "
			"this : %p, "
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

	bool bFlag = false;

	char pkt[RTP_MAX_LEN] = {0};
	unsigned int pktSize = size;

	if( DtlsSession::IsDTLS(data, size) ) {
		bFlag = mDtlsSession.RecvFrame(data, size);
		if( bFlag ) {
			// Check Handshake status
			DtlsSessionStatus status = mDtlsSession.GetDtlsSessionStatus();
			if( status == DtlsSessionStatus_HandshakeDone ) {
				LogAync(
						LOG_WARNING,
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
					mDtlsSession.GetClientKey(localKey, localSize);
					char remoteKey[SRTP_MASTER_LENGTH];
					int remoteSize = 0;
					mDtlsSession.GetServerKey(remoteKey, remoteSize);

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
				mRtpSession.Stop();
				mRtpDstAudioClient.Stop();
				mRtpDstVideoClient.Stop();

				if( mpWebRTCCallback ) {
					mpWebRTCCallback->OnWebRTCClose(this);
				}
			}
		}
	} else if( RtpSession::IsRtp(data, size) ) {
		bFlag = mRtpSession.RecvRtpPacket(data, size, pkt, pktSize);
		if( bFlag ) {
			int ssrc = RtpSession::GetRtpSSRC(pkt, pktSize);
			LogAync(
					LOG_STAT,
					"WebRTC::OnIceRecvData( "
					"this : %p, "
					"[Relay RTP], "
					"ssrc : %u, "
					"mAudioSSRC : %u, "
					"mVideoSSRC : %u "
					")",
					this,
					ssrc,
					mAudioSSRC,
					mVideoSSRC
					);

			if( ssrc == mAudioSSRC ) {
				mRtpDstAudioClient.SendRtpPacket(pkt, pktSize);
			} else if ( ssrc == mVideoSSRC ) {
				mRtpDstVideoClient.SendRtpPacket(pkt, pktSize);
			}
		}
	} else if( RtpSession::IsRtcp(data, size) ){
		bFlag = mRtpSession.RecvRtcpPacket(data, size, pkt, pktSize);
		if( bFlag ) {
			int ssrc = RtpSession::GetRtcpSSRC(pkt, pktSize);
			LogAync(
					LOG_STAT,
					"WebRTC::OnIceRecvData( "
					"this : %p, "
					"[Relay RTCP], "
					"ssrc : %u, "
					"mAudioSSRC : %u, "
					"mVideoSSRC : %u "
					")",
					this,
					ssrc,
					mAudioSSRC,
					mVideoSSRC
					);

			if( ssrc == mAudioSSRC ) {
				mRtpDstAudioClient.SendRtcpPacket(pkt, pktSize);
			} else if ( ssrc == mVideoSSRC ) {
				mRtpDstVideoClient.SendRtcpPacket(pkt, pktSize);
			}
		}
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
			LOG_MSG,
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
} /* namespace mediaserver */
