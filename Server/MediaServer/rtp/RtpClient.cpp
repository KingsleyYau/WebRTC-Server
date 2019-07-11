/*
 * RtpClient.cpp
 *
 *  Created on: 2019/06/20
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "RtpClient.h"

// Common
#include <common/LogManager.h>
#include <common/Math.h>
#include <common/CommonFunc.h>

// libsrtp
#include <srtp.h>
#include <srtp_priv.h>

/*
 * RTP_HEADER_LEN indicates the size of an RTP header
 */
#define RTP_HEADER_LEN 12
#define MAX_KEY_LEN 96
/*
 * RTP_MAX_BUF_LEN defines the largest RTP packet in the rtp.c implementation
 */
//#define RTP_MAX_BUF_LEN 16384
#define MTU 1514
#define RTP_HEADER_SIZE 54
#define RTP_MAX_BUF_LEN (MTU - RTP_HEADER_SIZE)
typedef struct {
    srtp_hdr_t header;
    char body[RTP_MAX_BUF_LEN];
} RtpClientMsg;

/* DTLS stuff */
#define DTLS_CIPHERS "ALL:NULL:eNULL:aNULL"

namespace mediaserver {
SSL_CTX *gpSSLCtx;
X509 *gpSSLCert;
EVP_PKEY *gpSSLKey;
unsigned char gFingerprint[EVP_MAX_MD_SIZE * 3];

void cb_dtls(const SSL *ssl, int where, int ret) {
}

int RtpClient::Generate_SSL_Keys(X509** certificate, EVP_PKEY** privateKey) {
	return 0;
}

bool RtpClient::Load_SSL_Keys(const char* server_pem, const char* server_key, X509** certificate, EVP_PKEY** privateKey) {
	bool bFlag = false;

    X509* cert = X509_new();
    BIO* bio_cert = BIO_new_file(server_pem, "rb");
    if( bio_cert ) {
        PEM_read_bio_X509(bio_cert, &cert, NULL, NULL);
        BIO_free(bio_cert);
        *certificate = cert;
    }

    EVP_PKEY* key = EVP_PKEY_new();
    BIO* bio_key = BIO_new_file(server_key, "rb");
    if( bio_key ) {
        PEM_read_bio_PrivateKey(bio_key, &key, NULL, NULL);
        BIO_free(bio_key);
        *privateKey = key;
    }

    if( *certificate && *privateKey ) {
    	bFlag = true;
    }

    return bFlag;
}

bool RtpClient::GobalInit() {
	bool bFlag = false;

	ERR_load_BIO_strings();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();

    /* initialize srtp library */
	srtp_err_status_t status = srtp_init();
	bFlag = (status == srtp_err_status_ok);
	if( bFlag ) {
		gpSSLCtx = SSL_CTX_new(DTLS_method());
		bFlag = (gpSSLCtx != NULL);
	}
	if( bFlag ) {
		// Load from disk
		bFlag = Load_SSL_Keys("./ssl/server.crt", "./ssl/server.key", &gpSSLCert, &gpSSLKey);
		// Generate new key
//		bFlag = (GenerateSSLKeys(&gpSSLCert, &gpSSLKey) == 0);
	}
	if( bFlag ) {
		bFlag = SSL_CTX_use_certificate(gpSSLCtx, gpSSLCert);
	}
	if( bFlag ) {
		bFlag = SSL_CTX_use_PrivateKey(gpSSLCtx, gpSSLKey);
	}
	if( bFlag ) {
		bFlag = SSL_CTX_check_private_key(gpSSLCtx);
	}
	if( bFlag ) {
		SSL_CTX_set_read_ahead(gpSSLCtx, 1);
		SSL_CTX_set_cipher_list(gpSSLCtx, DTLS_CIPHERS);
	}
	if( bFlag ) {
		memset(gFingerprint, 0, sizeof(gFingerprint));
	    unsigned int size;
	    unsigned char fingerprint[EVP_MAX_MD_SIZE];
	    bFlag = X509_digest(gpSSLCert, EVP_sha256(), (unsigned char *)fingerprint, &size);
	    char *c = (char *)&gFingerprint;
	    for(int i = 0; i < size; i++) {
	    	sprintf(c, "%.2X:", fingerprint[i]);
	    	c += 3;
	    }
	    if( bFlag ) {
	    	*(c - 1) = 0;
	    }
	}

	LogAync(
			LOG_ERR_USER,
			"RtpClient::GobalInit( "
			"[%s], "
			"SSL-Version : %s, "
			"SSL-Error : %s, "
			"gFingerPrint : %s "
			")",
			bFlag?"Success":"Fail",
			OpenSSL_version(OPENSSL_VERSION),
			ERR_reason_error_string(ERR_get_error()),
			gFingerprint
			);

	return bFlag;
}

RtpClient::RtpClient():
		mClientMutex(KMutex::MutexType_Recursive) {
	// TODO Auto-generated constructor stub
	Reset();
}

RtpClient::~RtpClient() {
	// TODO Auto-generated destructor stub
    if (mpSps) {
        delete[] mpSps;
        mpSps = NULL;
    }
    mSpsSize = 0;

    if (mpPps) {
        delete[] mpPps;
        mpPps = NULL;
    }
    mPpsSize = 0;

    if (mpSSL) {
        SSL_free(mpSSL);
        mpSSL = NULL;
    }
}

void RtpClient::Reset() {
	// Status
	mRunning = false;

	// Socket
	mpSocketSender = NULL;

	// Video
	mVideoTimestamp = 0;
	mVideoRtpTimestamp = 0;
	mVideoRtpSeq = 0;
	mNaluHeaderSize = 0;
	mInputVideoTimestamp = 0;
	mSendVideoFrameTimestamp = 0;
	mVideoFrameCount = 0;

	// Audio
	mAudioTimestamp = 0;
	mAudioRtpTimestamp = 0;
	mAudioRtpSeq = 0;

	// libsrtp
	/* ssrc value hardcoded for now */
	mSSRC = 0xDEADBEEF;

	mpSps = NULL;
	mSpsSize = 0;
	mpPps = NULL;
	mPpsSize = 0;

	// libopenssl
	mpSSL = NULL;
}

void RtpClient::SetSocketSender(SocketSender *sender) {
	mpSocketSender = sender;
}

bool RtpClient::Init() {
	bool bFlag = true;

	srtp_err_status_t status = srtp_err_status_ok;

	mClientMutex.lock();
	if( mRunning ) {
		Destroy();
	}

	mRunning = true;

    if( bFlag ) {
    	// Initialize SSL and BIO
    	mpSSL = SSL_new(gpSSLCtx);
    	SSL_set_ex_data(mpSSL, 0, this);
    	SSL_set_info_callback(mpSSL, cb_dtls);

    	mpReadBIO = BIO_new(BIO_s_mem());
    	BIO_set_mem_eof_return(mpReadBIO, -1);
    	mpWriteBIO = BIO_new(BIO_s_mem());
    	BIO_set_mem_eof_return(mpWriteBIO, -1);

    	SSL_set_bio(mpSSL, mpReadBIO, mpWriteBIO);
    	SSL_set_connect_state(mpSSL);
//    	SSL_set_accept_state(mpSSL);
    	EC_KEY *ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    	SSL_set_options(mpSSL, SSL_OP_SINGLE_ECDH_USE);
    	SSL_set_tmp_ecdh(mpSSL, ecdh);
    	EC_KEY_free(ecdh);

    	// Initialize libsrtp
    	srtp_policy_t policy;
    	memset(&policy, 0x0, sizeof(srtp_policy_t));

        /*
         * we're not providing security services, so set the policy to the
         * null policy
         *
         * Note that this policy does not conform to the SRTP
         * specification, since RTCP authentication is required.  However,
         * the effect of this policy is to turn off SRTP, so that this
         * application is now a vanilla-flavored RTP application.
         */
    	char key[MAX_KEY_LEN] = {0};

        srtp_crypto_policy_set_null_cipher_hmac_null(&policy.rtp);
        srtp_crypto_policy_set_null_cipher_hmac_null(&policy.rtcp);

        policy.key = (uint8_t *)key;
        policy.ssrc.type = ssrc_specific;
        policy.ssrc.value = mSSRC;
        policy.window_size = 0;
        policy.allow_repeat_tx = 0;
        policy.ekt = NULL;
        policy.next = NULL;

    	status = srtp_create(&mpSrtpCtx, &policy);
    	bFlag = (status == srtp_err_status_ok);
    }

	if( bFlag ) {
		LogAync(
				LOG_MSG,
				"RtpClient::Init( "
				"this : %p, "
				"[OK] "
				")",
				this
				);
	} else {
		LogAync(
				LOG_ERR_SYS,
				"RtpClient::Init( "
				"this : %p, "
				"[Fail], "
				"status : %d "
				")",
				this,
				status
				);
		Destroy();
	}

	mClientMutex.unlock();

	return bFlag;
}

void RtpClient::Destroy() {
	LogAync(
			LOG_WARNING,
			"RtpClient::Destroy( "
			"this : %p "
			")",
			this
			);

	mClientMutex.lock();
	if( mRunning ) {
		mRunning = false;

		if( mpSrtpCtx ) {
			srtp_dealloc(mpSrtpCtx);
			mpSrtpCtx = NULL;
		}

	    if (mpSps) {
	        delete[] mpSps;
	        mpSps = NULL;
	    }
	    mSpsSize = 0;

	    if (mpPps) {
	        delete[] mpPps;
	        mpPps = NULL;
	    }
	    mPpsSize = 0;

		Reset();
	}
	mClientMutex.unlock();
}

bool RtpClient::Handshake() {
	bool bFlag = true;

	LogAync(
			LOG_MSG,
			"RtpClient::Handshake( "
			"this : %p "
			")",
			this
			);

	int ret = SSL_do_handshake(mpSSL);
//	int ret = SSL_connect(mpSSL);
	bFlag = CheckHandshake();

	return bFlag;
}

void RtpClient::SetKeyFrameInfo(const char *sps, int spsSize, const char *pps, int ppsSize, int naluHeaderSize, u_int32_t timestamp) {
	mNaluHeaderSize = naluHeaderSize;

    bool bSpsChange = true;
    if (mpSps != NULL) {
        if (mSpsSize == spsSize) {
            if (0 != memcmp(mpSps, sps, spsSize)) {
                bSpsChange = true;

            } else {
                bSpsChange = false;
            }
        }
    }

    if (bSpsChange) {
        if (mpSps) {
            delete[] mpSps;
            mpSps = NULL;
        }

        mSpsSize = spsSize;
        mpSps = new char[mSpsSize];
        memcpy(mpSps, sps, mSpsSize);
    }

    bool bPpsChange = true;
    if (mpPps != NULL) {
        if (mPpsSize == ppsSize) {
            if (0 != memcmp(mpPps, pps, ppsSize)) {
                bPpsChange = true;

            } else {
                bPpsChange = false;
            }
        }
    }

    if (bPpsChange) {
        if (mpPps) {
            delete[] mpPps;
            mpPps = NULL;
        }

        mPpsSize = ppsSize;
        mpPps = new char[mPpsSize];
        memcpy(mpPps, pps, mPpsSize);
    }

    if( bSpsChange || bPpsChange ) {
    	LogAync(
    			LOG_STAT,
    			"RtpClient::SetKeyFrameInfo( "
    			"this : %p, "
    			"timestamp : %u, "
				"mSpsSize : %d, "
    			"mPpsSize : %d, "
    			"mNaluHeaderSize : %d "
    			")",
    			this,
    			timestamp,
				mSpsSize,
				mPpsSize,
				mNaluHeaderSize
    			);
    }
}

bool RtpClient::SendVideoFrame(const char* frame, unsigned int size, unsigned int timestamp) {
	bool bFlag = true;

	srtp_err_status_t status = srtp_err_status_ok;

	mClientMutex.lock();
	if( mRunning ) {
        // 计算RTP时间戳
        int sendTimestamp = 0;

        // 第一帧
        if (mInputVideoTimestamp == 0) {
        	mInputVideoTimestamp = timestamp;
        }

        // 当前帧比上一帧时间戳大, 计算时间差
        if (timestamp > mInputVideoTimestamp) {
            sendTimestamp = timestamp - mInputVideoTimestamp;
        }

        mSendVideoFrameTimestamp += sendTimestamp * 90;
        mInputVideoTimestamp = timestamp;

        SendVideoKeyFrame();

		RtpClientMsg msg;
		int lastSize = 0;
		int bodySize = 0;
		int pktSize = 0;

	    Nalu naluArray[16];
	    int naluArraySize = _countof(naluArray);
	    bool bFlag = mVideoMuxer.GetNalus(frame, size, mNaluHeaderSize, naluArray, naluArraySize);
	    if( bFlag && naluArraySize > 0 ) {
	        int naluIndex = 0;
	        while( naluIndex < naluArraySize ) {
	            Nalu* nalu = naluArray + naluIndex;
	            naluIndex++;

	        	const char *body = nalu->GetNaluBody();
	        	lastSize = nalu->GetNaluBodySize();
	        	if( lastSize <= sizeof(msg.body) ) {
	        		// Send single NALU with one RTP packet
		        	LogAync(
		        			LOG_STAT,
		        			"RtpClient::SendVideoFrame( "
		        			"this : %p, "
							"[Send single NALU with single RTP packet], "
		        			"timestamp : %u, "
							"naluType : %d, "
							"naluBodySize : %d, "
							"[Mark] "
		        			")",
							this,
							mSendVideoFrameTimestamp,
							nalu->GetNaluType(),
							nalu->GetNaluBodySize()
		        			);

	    			msg.header.ssrc = htonl(mSSRC);
	    			msg.header.seq = htons(mVideoRtpSeq++);
	    			msg.header.ts = htonl(mSendVideoFrameTimestamp);
	    			msg.header.m = 1;
	    			msg.header.pt = 96;
	    			msg.header.version = 2;
	    			msg.header.p = 0;
	    			msg.header.x = 0;
	    			msg.header.cc = 0;

	    			bodySize = lastSize;
	    			// NALU payload
	    			memcpy(msg.body, body, bodySize);

	    			pktSize = RTP_HEADER_LEN + bodySize;
	    			status = srtp_protect(mpSrtpCtx, &msg.header, &pktSize);
	    		    if (status == srtp_err_status_ok) {
	    		    	if( mpSocketSender ) {
	    		    		int sendSize = mpSocketSender->SendData((void *)&msg, pktSize);
							if (sendSize != pktSize) {
								bFlag = false;
							}
	    		    	} else {
	    		    		bFlag = false;
	    		    	}
//						int sendSize = sendto(mFd, (void *)&msg, pktSize, 0, (struct sockaddr *)&mSendSockAddr, sizeof(struct sockaddr_in));
//						if (sendSize != pktSize) {
//							bFlag = false;
//						}
	    		    } else {
	    		    	bFlag = false;
	    		    }

	        	} else {
	        		// Send single NALU with multiple RTP packets
		        	LogAync(
		        			LOG_STAT,
		        			"RtpClient::SendVideoFrame( "
		        			"this : %p, "
							"[Send single NALU with multiple RTP packets], "
		        			"timestamp : %u, "
							"naluType : %d, "
							"naluBodySize: %d "
		        			")",
							this,
							mSendVideoFrameTimestamp,
							nalu->GetNaluType(),
							nalu->GetNaluBodySize()
		        			);

		        	// Wipe off original NALU header
		        	char naluHeader = body[0];

		            Slice* sliceArray = NULL;
		            int sliceArraySize = 0;
		            int sliceIndex = 0;

					nalu->GetSlices(&sliceArray, sliceArraySize);
					while( sliceIndex < sliceArraySize ) {
						Slice* slice = sliceArray + sliceIndex;
						sliceIndex++;

			        	const char *body = slice->GetSlice();
			        	lastSize = slice->GetSliceSize();
			        	lastSize--;
			        	body++;
			        	int packetIndex = 0;
			        	int sentSize = 0;

			        	while( true ) {
			        		// 2 bytes for FU indicator and FU header
			        		bodySize = MIN(lastSize, sizeof(msg.body) - 2);

			    			msg.header.ssrc = htonl(mSSRC);
			    			msg.header.seq = htons(mVideoRtpSeq++);
			    			msg.header.ts = htonl(mSendVideoFrameTimestamp);
			    			msg.header.m = (lastSize <= bodySize)?1:0;
			    			msg.header.pt = 96;
			    			msg.header.version = 2;
			    			msg.header.p = 0;
			    			msg.header.x = 0;
			    			msg.header.cc = 0;

		    		    	// Hard code here, cause 0x7C means important, 0x5C means not
		    		    	/**
		    		    	 * FU indicator
		    		    	 * Hard code here, cause 0x7C means important, 0x5C means not
		    		    	 * [F].[NRI].[  TYPE   ]
		    		    	 * [7].[6.5].[4.3.2.1.0]
		    		    	 * 28 : FU-A
		    		    	 */
		    		    	msg.body[0] = 0x60 | 28;
		    		    	/**
		    		    	 * FU header
		    		    	 * [S].[E].[R].[  TYPE   ]
		    		    	 * [7].[6].[5].[4.3.2.1.0]
		    		    	 */
							msg.body[1] = nalu->GetNaluType();
		    		    	if( packetIndex == 0 && !msg.header.m ) {
		    		    		// Start packet
		    		    		msg.body[1] |= 0x80;
		    		    	} else if( msg.header.m ) {
		    		    		// Last packet
		    		    		msg.body[1] |= 0x40;
		    		    	} else {
		    		    		// Middle packet
		    		    	}

		    		    	// FU payload
							memcpy(msg.body + 2, body + sentSize, bodySize);

			    			// Whole size
			    			pktSize = RTP_HEADER_LEN + 2 + bodySize;
			    			status = srtp_protect(mpSrtpCtx, &msg.header, &pktSize);
			    		    if (status == srtp_err_status_ok) {
			    		    	int sendSize = 0;
			    		    	if( mpSocketSender ) {
			    		    		sendSize = mpSocketSender->SendData((void *)&msg, pktSize);
			    		    	}
//								int sendSize = sendto(mFd, (void *)&msg, pktSize, 0, (struct sockaddr *)&mSendSockAddr, sizeof(struct sockaddr_in));

								unsigned char t = nalu->GetNaluType();
								unsigned char d = msg.body[0];
								unsigned char h = msg.body[1];

					        	LogAync(
					        			LOG_STAT,
										"RtpClient::SendVideoFrame( "
										"this : %p, "
										"[Send FUs packet], "
										"timestamp : %u, "
										"NALU-T : 0x%02x, "
										"FU-D : 0x%02x, "
										"FU-H : 0x%02x, "
										"pktSize : %d "
										"%s"
										")",
										this,
										mSendVideoFrameTimestamp,
										t,
										d,
										h,
										pktSize,
										msg.header.m?", [Mark] ":""
										);

								if (sendSize == pktSize) {
									sentSize += bodySize;
									lastSize -= bodySize;
									packetIndex++;

									if( lastSize == 0 ) {
										// All data has been sent
										break;
									}
								} else {
									bFlag = false;
									break;
								}

			    		    } else {
			    		    	bFlag = false;
			    		    }
			        	}
					}
	        	}
	        }
	    } else {
	    	bFlag = false;
	    }
	} else {
		bFlag = false;
	}
	mClientMutex.unlock();

//	LogAync(
//			LOG_WARNING,
//			"RtpClient::SendVideoFrame( "
//			"this : %p, "
//			"[%s], "
//			"timestamp : %d, "
//			"size : %d, "
//			"status : %d "
//			")",
//			this,
//			bFlag?"Success":"Fail",
//			timestamp,
//			size,
//			status
//			);

	return bFlag;
}

bool RtpClient::SendVideoKeyFrame() {
	bool bFlag = true;

	srtp_err_status_t status = srtp_err_status_ok;
	RtpClientMsg msg;
	int lastSize = 0;
	int bodySize = 0;
	int pktSize = 0;

	mClientMutex.lock();
	if( mRunning ) {
		if( mVideoFrameCount++ % 10 == 0 ) {
	    	LogAync(
	    			LOG_STAT,
	    			"RtpClient::SendVideoKeyFrame( "
	    			"this : %p, "
					"[Send SPS/PPS RTP packet], "
	    			"timestamp : %u "
	    			")",
					this,
					mSendVideoFrameTimestamp
	    			);

			msg.header.ssrc = htonl(mSSRC);
			msg.header.seq = htons(mVideoRtpSeq++);
			msg.header.ts = htonl(mSendVideoFrameTimestamp);
			msg.header.m = 0;
			msg.header.pt = 96;
			msg.header.version = 2;
			msg.header.p = 0;
			msg.header.x = 0;
			msg.header.cc = 0;


	    	// Hard code here, cause 0x7C means important, 0x5C means not
	    	/**
	    	 * RTP payload header
	    	 * [F].[NRI].[  TYPE   ]
	    	 * [7].[6.5].[4.3.2.1.0]
	    	 * 24(0x18) : STAP-A
	    	 */
			bodySize = 0;
			msg.body[bodySize++] = 0x18;
			// sps size 2 bytes
			short spsLength = htons(mSpsSize);
			memcpy(msg.body + bodySize, (const void *)&spsLength, mSpsSize);
			bodySize += sizeof(spsLength);
			// sps data
			memcpy(msg.body + bodySize, mpSps, mSpsSize);
			bodySize += mSpsSize;

			// pps size 2 bytes
			short ppsLength = htons(mPpsSize);
			memcpy(msg.body + bodySize, (const void *)&ppsLength, sizeof(ppsLength));
			bodySize += sizeof(ppsLength);
			// pps data
			memcpy(msg.body + bodySize, mpPps, mPpsSize);
			bodySize += mPpsSize;

			pktSize = RTP_HEADER_LEN + bodySize;
			status = srtp_protect(mpSrtpCtx, &msg.header, &pktSize);
		    if (status == srtp_err_status_ok) {
		    	if( mpSocketSender ) {
		    		int sendSize = mpSocketSender->SendData((void *)&msg, pktSize);
					if (sendSize != pktSize) {
						bFlag = false;
					}
		    	} else {
		    		bFlag = false;
		    	}
//				int sendSize = sendto(mFd, (void *)&msg, pktSize, 0, (struct sockaddr *)&mSendSockAddr, sizeof(struct sockaddr_in));
//				if (sendSize != pktSize) {
//					bFlag = false;
//				}
		    } else {
		    	bFlag = false;
		    }
		}

	}
	return bFlag;
}

bool RtpClient::SendAudioFrame(const char* frame, unsigned int size, unsigned int timestamp) {
	bool bFlag = false;

	LogAync(
			LOG_WARNING,
			"RtpClient::SendAudioFrame( "
			"this : %p, "
			"timestamp : %d, "
			"size : %u "
			")",
			this,
			timestamp,
			size
			);

	return bFlag;
}

bool RtpClient::RecvFrame(const char* frame, unsigned int size) {
	bool bFlag = false;

	LogAync(
			LOG_WARNING,
			"RtpClient::RecvFrame( "
			"this : %p, "
			"size : %d "
			")",
			this,
			size
			);

	int written = BIO_write(mpReadBIO, frame, size);
	LogAync(
			LOG_WARNING,
			"RtpClient::RecvFrame( "
			"this : %p, "
			"written : %d "
			")",
			this,
			written
			);
    char data[1500];
    memset(&data, 0, 1500);
	int read = SSL_read(mpSSL, &data, 1500);
	LogAync(
			LOG_WARNING,
			"RtpClient::RecvFrame( "
			"this : %p, "
			"read : %d "
			")",
			this,
			read
			);
	if ( read < 0 ) {
	    unsigned long ret = SSL_get_error(mpSSL, read);
	    if ( ret == SSL_ERROR_SSL ) {
	        char error_string[200];
	        ERR_error_string_n(ERR_get_error(), error_string, 200);
	    	LogAync(
	    			LOG_WARNING,
	    			"RtpClient::RecvFrame( "
	    			"this : %p, "
					"[Handshake Error], "
					"error : %d, "
	    			"error_string : %s "
	    			")",
	    			this,
					ERR_get_error(),
					error_string
	    			);
	    }
	}

	bFlag = CheckHandshake();

	if ( SSL_is_init_finished(mpSSL) ) {
		X509 *cert = SSL_get_peer_certificate(mpSSL);

		unsigned char remoteFingerprint[EVP_MAX_MD_SIZE * 3];
		memset(remoteFingerprint, 0, sizeof(remoteFingerprint));
	    unsigned int fingerprintSize;
	    unsigned char fingerprint[EVP_MAX_MD_SIZE];
	    bFlag = X509_digest(cert, EVP_sha256(), (unsigned char *)fingerprint, &fingerprintSize);
        X509_free(cert);

	    char *c = (char *)&remoteFingerprint;
	    for(int i = 0; i < fingerprintSize; i++) {
	    	sprintf(c, "%.2X:", fingerprint[i]);
	    	c += 3;
	    }
	    if( bFlag ) {
	    	*(c - 1) = 0;
	    }

		LogAync(
				LOG_WARNING,
				"RtpClient::RecvFrame( "
				"this : %p, "
				"[SSL_is_init_finished], "
				"remoteFingerprint : %s "
				")",
				this,
				remoteFingerprint
				);

        /* Complete with SRTP setup */
        unsigned char material[SRTP_MASTER_LENGTH*2];
        unsigned char *local_key, *local_salt, *remote_key, *remote_salt;
        /* Export keying material for SRTP */
        if (!SSL_export_keying_material(ssl, material, SRTP_MASTER_LENGTH*2, "EXTRACTOR-dtls_srtp", 19, NULL, 0, 0))
        {

        }
	}

	return bFlag;
}

bool RtpClient::CheckHandshake() {
	bool bFlag = true;

	int pending = BIO_ctrl_pending(mpWriteBIO);
	LogAync(
			LOG_MSG,
			"RtpClient::CheckHandshake( "
			"this : %p, "
			"pending : %d "
			")",
			this,
			pending
			);

	char dataBuffer[1500] = {0};
	int dataSize = 0;
	while (pending > 0) {
		dataSize = MIN(pending, 1500);
		int pktSize = BIO_read(mpWriteBIO, dataBuffer, dataSize);
		LogAync(
				LOG_MSG,
				"RtpClient::CheckHandshake( "
				"this : %p, "
				"[Send DTLS data, %d bytes] "
				")",
				this,
				pktSize
				);

    	if( mpSocketSender ) {
    		int sendSize = mpSocketSender->SendData((void *)dataBuffer, pktSize);
			if (sendSize != pktSize) {
				bFlag = false;
			}
    	} else {
    		bFlag = false;
    	}

		/* Check if there's anything left to send (e.g., fragmented packets) */
		pending = BIO_ctrl_pending(mpWriteBIO);
	}

	return bFlag;
}

} /* namespace mediaserver */
