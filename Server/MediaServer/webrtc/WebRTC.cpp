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

void WebRTC::SetRemoteSdp(const string& sdp) {
	LogAync(
			LOG_WARNING,
			"WebRTC::SetRemoteSdp( "
			"this : %p, "
			"sdp : \n%s"
			")",
			this,
			sdp.c_str()
			);

	struct sdp_session *session = NULL;
	int err = -1;
	err = sdp_description_read(sdp.c_str(), &session);
	if( err == 0 ) {
		sdp_media *media = NULL;
		list_node node;

		LogAync(
				LOG_MSG,
				"WebRTC::SetRemoteSdp( "
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
						"WebRTC::SetRemoteSdp( "
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
				"WebRTC::SetRemoteSdp( "
				"this : %p, "
				"mAudioSSRC : %u, "
				"mVideoSSRC : %u "
				")",
				this,
				mAudioSSRC,
				mVideoSSRC
				);
	}

	mIceClient.SetRemoteSdp(sdp);

}

bool WebRTC::Start() {
	bool bFlag = true;

	bFlag &= mIceClient.Start();
	bFlag &= mDtlsSession.Start();

	mRtpDstAudioClient.Init("192.168.88.138", 9999, 9999);
	mRtpDstAudioClient.Start(NULL, 0, NULL, 0);
	mRtpDstVideoClient.Init("192.168.88.138", 10001, 10001);
	mRtpDstVideoClient.Start(NULL, 0, NULL, 0);

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
	mIceClient.Stop();
	mDtlsSession.Stop();
	mRtpSession.Stop();

	LogAync(
			LOG_WARNING,
			"WebRTC::Stop( "
			"this : %p, "
			"[OK] "
			")",
			this
			);
}

int WebRTC::SendData(const void *data, unsigned int len) {
	// Send RTP data through ICE data channel
	return mIceClient.SendData(data, len);
}

void WebRTC::OnIceHandshakeFinish(IceClient *ice) {
	LogAync(
			LOG_MSG,
			"WebRTC::OnIceHandshakeFinish( "
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
//			LOG_MSG,
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

} /* namespace mediaserver */
