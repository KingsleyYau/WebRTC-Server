/*
 * WebRTC.cpp
 *
 *  Created on: 2019/07/02
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "WebRTC.h"

// Common
#include <common/LogManager.h>

// ThirdParty
#include <libsdp.h>

namespace mediaserver {

WebRTC::WebRTC() {
	// TODO Auto-generated constructor stub
	mIceClient.SetCallback(this);
	mDtlsSession.SetSocketSender(this);
	mRtpSession.SetRtpSender(this);
	mRtpSession.SetRtcpSender(this);

	mVideoSSRC = 0;
	mAudioSSRC = 0;

	mpWebRTCCallback = NULL;
	mpCustom = NULL;
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

bool WebRTC::ParseRemoteSdp(const string& sdp) {
	LogAync(
			LOG_WARNING,
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

		sdp_media *media = NULL;
		list_node node;

		LogAync(
				LOG_MSG,
				"WebRTC::ParseRemoteSdp( "
				"this : %p, "
				"media_count : %d "
				")",
				this,
				session->media_count
				);
		list_walk_entry_forward(&session->medias, media, node) {
			if ( media ) {
				LogAync(
						LOG_MSG,
						"WebRTC::ParseRemoteSdp( "
						"this : %p, "
						"media_type : %s, "
						"media_attr_count : %d "
						")",
						this,
						sdp_media_type_str(media->type),
						media->attr_count
						);
				sdp_attr *attr = NULL;
				list_walk_entry_forward(&media->attrs, attr, node) {
					if( attr->key && attr->value ) {
						string key(attr->key);
						string value(attr->value);

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
						}
					}
				}
			}
		}

		LogAync(
				LOG_MSG,
				"WebRTC::ParseRemoteSdp( "
				"this : %p, "
				"mAudioSSRC : %u, "
				"mVideoSSRC : %u "
				")",
				this,
				mAudioSSRC,
				mVideoSSRC
				);
	}

	if( bFlag ) {
		mIceClient.SetRemoteSdp(sdp);
	}

	return bFlag;
}

bool WebRTC::Start(const string& sdp) {
	bool bFlag = true;

//	bFlag &= ParseRemoteSdp(sdp);
	ParseRemoteSdp(sdp);
	bFlag &= mIceClient.Start();
	bFlag &= mDtlsSession.Start();

	bFlag &= mRtpDstAudioClient.Init("192.168.88.138", 9999, 9999);
	bFlag &= mRtpDstAudioClient.Start(NULL, 0, NULL, 0);
	bFlag &= mRtpDstVideoClient.Init("192.168.88.138", 10001, 10001);
	bFlag &= mRtpDstVideoClient.Start(NULL, 0, NULL, 0);

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

	mIceClient.Stop();
	mDtlsSession.Stop();
	mRtpSession.Stop();
	mRtpDstAudioClient.Stop();
	mRtpDstVideoClient.Stop();

	LogAync(
			LOG_WARNING,
			"WebRTC::Stop( "
			"this : %p, "
			"[OK] "
			")",
			this
			);
}

void WebRTC::SetCallback(WebRTCCallback *callback) {
	mpWebRTCCallback = callback;
}

void WebRTC::SetCustom(void *custom) {
	mpCustom = custom;
}

void* WebRTC::GetCustom() {
	return mpCustom;
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

	char sdp[4096];
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
			"a=ice-ufrag:%s\n"
			"a=ice-pwd:%s\n"
			"a=candidate:4 1 UDP 335544831 192.168.88.133 %u typ %s raddr %s rport 9\n"
			"a=ice-options:trickle\n"
			"a=fingerprint:sha-256 %s\n"
			"a=setup:active\n"
			"a=mid:0\n"
			"a=recvonly\n"
			"a=rtcp-mux\n"
			"a=rtpmap:111 opus/48000/2\n"
			"a=rtcp-fb:111 transport-cc\n"
			"a=fmtp:111 minptime=10;useinbandfec=1\n"
			"m=video 9 UDP/TLS/RTP/SAVPF 100\n"
			"c=IN IP4 %s\n"
			"a=rtcp:9 IN IP4 0.0.0.0\n"
			"a=ice-ufrag:%s\n"
			"a=ice-pwd:%s\n"
			"a=ice-options:trickle\n"
			"a=fingerprint:sha-256 %s\n"
			"a=setup:active\n"
			"a=mid:1\n"
			"a=recvonly\n"
			"a=rtcp-mux\n"
			"a=rtcp-rsize\n"
			"a=rtpmap:100 VP8/90000\n",
//			"a=rtpmap:107 H264/90000\n"
//			"a=rtcp-fb:107 goog-remb\n"
//			"a=rtcp-fb:107 transport-cc\n"
//			"a=rtcp-fb:107 ccm fir\n"
//			"a=rtcp-fb:107 nack\n"
//			"a=rtcp-fb:107 nack pli\n"
//			"a=fmtp:107 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42001f\n",
			port,
			ip.c_str(),
			ufrag.c_str(),
			pwd.c_str(),
			port,
			type.c_str(),
			ip.c_str(),
			DtlsSession::GetFingerprint(),
			ip.c_str(),
			ufrag.c_str(),
			pwd.c_str(),
			DtlsSession::GetFingerprint()
			);

	if( mpWebRTCCallback ) {
		mpWebRTCCallback->OnWebRTCCreateSdp(this, sdp);
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
//	LogAync(
//			LOG_STAT,
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
				char localKey[SRTP_MASTER_LENGTH];
				int localSize = 0;
				mDtlsSession.GetClientKey(localKey, localSize);
				char remoteKey[SRTP_MASTER_LENGTH];
				int remoteSize = 0;
				mDtlsSession.GetServerKey(remoteKey, remoteSize);

				mRtpSession.Start(localKey, localSize, remoteKey, remoteSize);
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
					"ssrc : %u "
					")",
					this,
					ssrc
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
					"ssrc : %u "
					")",
					this,
					ssrc
					);

			if( ssrc == mAudioSSRC ) {
				mRtpDstAudioClient.SendRtcpPacket(pkt, pktSize);
			} else if ( ssrc == mVideoSSRC ) {
				mRtpDstVideoClient.SendRtcpPacket(pkt, pktSize);
			}
		}
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

} /* namespace mediaserver */
