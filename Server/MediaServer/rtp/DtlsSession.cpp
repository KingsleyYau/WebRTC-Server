/*
 * DtlsSession.cpp
 *
 *  Created on: 2019/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include <include/CommonHeader.h>

// Common
#include <common/CommonFunc.h>
#include <common/LogManager.h>
#include <common/Math.h>

// System
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "DtlsSession.h"

#define DTLS_AUTOCERT_DURATION 60 * 60 * 24 * 365
#define MTU 1500

namespace mediaserver {
SSL_CTX *gpSSLCtx;
X509 *gpSSLCert;
EVP_PKEY *gpSSLKey;
unsigned char gFingerprint[EVP_MAX_MD_SIZE * 3];

int DtlsSession::SSL_Generate_Keys(X509** certificate, EVP_PKEY** privateKey) {
	bool bFlag = true;

	LogAync(
			LOG_INFO,
			"DtlsSession::SSL_Generate_Keys("
			")"
			);

    static const int num_bits = 2048;
    BIGNUM* bne = NULL;
    RSA* rsa_key = NULL;
    X509_NAME* cert_name = NULL;

    /* Create a big number object. */
    bne = BN_new();
    if ( !bne ) {
    	LogAync(
    			LOG_INFO,
    			"DtlsSession::SSL_Generate_Keys( "
				"[BN_new() Fail] "
    			")"
    			);
        bFlag = false;
    }

    if( bFlag ) {
        if (!BN_set_word(bne, RSA_F4)) {
        	/* RSA_F4 == 65537 */
        	LogAync(
        			LOG_INFO,
        			"DtlsSession::SSL_Generate_Keys( "
    				"[BN_set_word() Fail] "
        			")"
        			);
            bFlag = false;
        }
    }

    if( bFlag ) {
        /* Generate a RSA key. */
        rsa_key = RSA_new();
        if ( !bFlag || !rsa_key ) {
        	LogAync(
        			LOG_INFO,
        			"DtlsSession::SSL_Generate_Keys( "
    				"[RSA_new() Fail] "
        			")"
        			);
            bFlag = false;
        }
    }

    if( bFlag ) {
		/* This takes some time. */
		if (!RSA_generate_key_ex(rsa_key, num_bits, bne, NULL)) {
        	LogAync(
        			LOG_INFO,
        			"DtlsSession::SSL_Generate_Keys( "
    				"[RSA_generate_key_ex() Fail] "
        			")"
        			);
			bFlag = false;
		}
    }

    if( bFlag ) {
		/* Create a private key object (needed to hold the RSA key). */
		*privateKey = EVP_PKEY_new();
		if (!*privateKey) {
        	LogAync(
        			LOG_INFO,
        			"DtlsSession::SSL_Generate_Keys( "
    				"[EVP_PKEY_new() Fail] "
        			")"
        			);
			bFlag = false;
		}
    }

    if( bFlag ) {
		if (!EVP_PKEY_assign_RSA(*privateKey, rsa_key)) {
        	LogAync(
        			LOG_INFO,
        			"DtlsSession::SSL_Generate_Keys( "
    				"[EVP_PKEY_assign_RSA() Fail] "
        			")"
        			);
			bFlag = false;
		}
    }

    if( bFlag ) {
		/* The RSA key now belongs to the private key, so don't clean it up separately. */
		rsa_key = NULL;

		/* Create the X509 certificate. */
		*certificate = X509_new();
		if (!*certificate) {
        	LogAync(
        			LOG_INFO,
        			"DtlsSession::SSL_Generate_Keys( "
    				"[X509_new() Fail] "
        			")"
        			);
			bFlag = false;
		}
    }

    if( bFlag ) {
		/* Set version 3 (note that 0 means version 1). */
		X509_set_version(*certificate, 2);

		/* Set serial number. */
		srand((unsigned)time(NULL));
		long seed = rand();
		ASN1_INTEGER_set(X509_get_serialNumber(*certificate), seed);

		/* Set valid period. */

		X509_gmtime_adj(X509_get_notBefore(*certificate), -1 * DTLS_AUTOCERT_DURATION);  /* -1 year */
		X509_gmtime_adj(X509_get_notAfter(*certificate), DTLS_AUTOCERT_DURATION);  /* 1 year */

		/* Set the public key for the certificate using the key. */
		if (!X509_set_pubkey(*certificate, *privateKey)) {
        	LogAync(
        			LOG_INFO,
        			"DtlsSession::SSL_Generate_Keys( "
    				"[X509_set_pubkey() Fail] "
        			")"
        			);
			bFlag = false;
		}
    }

    if( bFlag ) {
		/* Set certificate fields. */
		cert_name = X509_get_subject_name(*certificate);
		if (!cert_name) {
        	LogAync(
        			LOG_INFO,
        			"DtlsSession::SSL_Generate_Keys( "
    				"[X509_get_subject_name() Fail] "
        			")"
        			);
			bFlag = false;
		}
	}

    if( bFlag ) {
		X509_NAME_add_entry_by_txt(cert_name, "O", MBSTRING_ASC, (const unsigned char*)"Janus", -1, -1, 0);
		X509_NAME_add_entry_by_txt(cert_name, "CN", MBSTRING_ASC, (const unsigned char*)"Janus", -1, -1, 0);

		/* It is self-signed so set the issuer name to be the same as the subject. */
		if (!X509_set_issuer_name(*certificate, cert_name)) {
        	LogAync(
        			LOG_INFO,
        			"DtlsSession::SSL_Generate_Keys( "
    				"[X509_set_issuer_name() Fail] "
        			")"
        			);
			bFlag = false;
		}
    }

    if( bFlag ) {
		/* Sign the certificate with the private key. */
		if (!X509_sign(*certificate, *privateKey, EVP_sha1())) {
        	LogAync(
        			LOG_INFO,
        			"DtlsSession::SSL_Generate_Keys( "
    				"[X509_sign() Fail] "
        			")"
        			);
			bFlag = false;
		}
    }

    if (bne) {
        BN_free(bne);
    }
//    if (rsa_key && !*privateKey) {
//        RSA_free(rsa_key);
//    }
//    if (*privateKey) {
//        EVP_PKEY_free(*privateKey);
//    }
//    if (*certificate) {
//        X509_free(*certificate);
//    }

	LogAync(
			LOG_INFO,
			"DtlsSession::SSL_Generate_Keys( "
			"[%s] "
			")",
			FLAG_2_STRING(bFlag)
			);

	return bFlag;
}

bool DtlsSession::SSL_Load_Keys(const char* server_pem, const char* server_key, X509** certificate, EVP_PKEY** privateKey) {
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

void DtlsSession::SSL_Info_Callback(const SSL* s, int where, int ret) {
	DtlsSession *client = (DtlsSession *)SSL_get_ex_data(s, 0);

	string str = "undefined";
	int w = where & ~SSL_ST_MASK;
	if (w & SSL_ST_CONNECT) {
		str = "SSL_connect";
	} else if (w & SSL_ST_ACCEPT) {
		str = "SSL_accept";
	} else if (w & SSL_CB_ALERT) {
		str = "SSL_alert";
		client->mDtlsSessionStatus = DtlsSessionStatus_Alert;
	} else if (where & SSL_CB_HANDSHAKE_START) {
		str = "SSL_start";
		client->mDtlsSessionStatus = DtlsSessionStatus_HandshakeStart;
	} else if (where & SSL_CB_HANDSHAKE_DONE) {
		str = "SSL_done";
		client->mDtlsSessionStatus = DtlsSessionStatus_HandshakeDone;
	}

	if (where & SSL_CB_LOOP) {
		str += ":";
		str += SSL_state_string_long(s);
	} else if (where & SSL_CB_ALERT) {
		str += ":";
		str += (where & SSL_CB_READ) ? "read" : "write";
		str += ":";
		str += SSL_alert_type_string_long(ret);
		str += ":";
		str += SSL_alert_desc_string_long(ret);
	} else if (where & SSL_CB_EXIT) {
		str += ":exit";
	}

	LogAync(
			LOG_INFO,
			"DtlsSession::SSL_Info_Callback( "
			"this : %p, "
			"[%s], "
			"where : 0x%x, "
			"ret : %d "
			")",
			client,
			str.c_str(),
			where,
			ret
			);
}

bool DtlsSession::GobalInit(const string& certPath, const string& keyPath) {
	bool bFlag = false;

	ERR_load_BIO_strings();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();

	gpSSLCtx = SSL_CTX_new(DTLS_method());
	bFlag = (gpSSLCtx != NULL);
	if( bFlag ) {
		// Set debug mode
//		SSL_CTX_set_info_callback(gpSSLCtx, DtlsSession::SSL_Info_Callback);
		// Load from disk
		bFlag = SSL_Load_Keys(certPath.c_str(), keyPath.c_str(), &gpSSLCert, &gpSSLKey);
		// Generate new key
//		bFlag = SSL_Generate_Keys(&gpSSLCert, &gpSSLKey);
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
		bFlag = (SSL_CTX_set_tlsext_use_srtp(gpSSLCtx, "SRTP_AES128_CM_SHA1_80") == 0);
	}
	if( bFlag ) {
	    // Enable read-ahead for DTLS so whole packets are read from internal BIO
	    // before parsing. This is done internally by BoringSSL for DTLS.
		SSL_CTX_set_read_ahead(gpSSLCtx, 1);
		// Select list of available ciphers. Note that !SHA256 and !SHA384 only
		// remove HMAC-SHA256 and HMAC-SHA384 cipher suites, not GCM cipher suites
		// with SHA256 or SHA384 as the handshake hash.
		// This matches the list of SSLClientSocketOpenSSL in Chromium.
		SSL_CTX_set_cipher_list(gpSSLCtx, "DEFAULT:!NULL:!aNULL:!SHA256:!SHA384:!aECDH:!AESGCM+AES256:!aPSK");
	}
	if( bFlag ) {
		memset(gFingerprint, 0, sizeof(gFingerprint));
	    unsigned int size;
	    unsigned char fingerprint[EVP_MAX_MD_SIZE];
	    bFlag = X509_digest(gpSSLCert, EVP_sha256(), (unsigned char *)fingerprint, &size);
	    char *c = (char *)&gFingerprint;
	    for(int i = 0; i < (int)size; i++) {
	    	sprintf(c, "%.2X:", fingerprint[i]);
	    	c += 3;
	    }
	    if( bFlag ) {
	    	*(c - 1) = 0;
	    }
	}

	if ( bFlag ) {
		LogAync(
				LOG_NOTICE,
				"DtlsSession::GobalInit( "
				"[%s], "
				"SSL-Version : %s, "
				"gFingerPrint : %s, "
				"certPath : %s, "
				"keyPath : %s "
				")",
				FLAG_2_STRING(bFlag),
				OpenSSL_version(OPENSSL_VERSION),
				gFingerprint,
				certPath.c_str(),
				keyPath.c_str()
				);
	} else {
		LogAync(
				LOG_ALERT,
				"DtlsSession::GobalInit( "
				"[%s], "
				"SSL-Version : %s, "
				"SSL-Error : %s, "
				"gFingerPrint : %s, "
				"certPath : %s, "
				"keyPath : %s "
				")",
				FLAG_2_STRING(bFlag),
				OpenSSL_version(OPENSSL_VERSION),
				ERR_reason_error_string(ERR_get_error()),
				gFingerprint,
				certPath.c_str(),
				keyPath.c_str()
				);
	}


	return bFlag;
}

const unsigned char *DtlsSession::GetFingerprint() {
	return gFingerprint;
}

DtlsSession::DtlsSession()
:mClientMutex(KMutex::MutexType_Recursive) {
	// TODO Auto-generated constructor stub
	mRunning = false;
	mpSocketSender = NULL;
	mDtlsSessionStatus = DtlsSessionStatus_None;

	// libopenssl
	mpSSL = NULL;
	mpReadBIO = NULL;
	mpWriteBIO = NULL;

	memset(mClientSalt, 0, sizeof(mClientSalt));
	memset(mServerKey, 0, sizeof(mServerKey));
}

DtlsSession::~DtlsSession() {
	// TODO Auto-generated destructor stub
}

void DtlsSession::SetSocketSender(SocketSender *sender) {
	mpSocketSender = sender;
}

bool DtlsSession::Start(bool bActive) {
	bool bFlag = true;

	LogAync(
			LOG_INFO,
			"DtlsSession::Start( "
			"this : %p, "
			"bActive : %s "
			")",
			this,
			BOOL_2_STRING(bActive)
			);

	mClientMutex.lock();
	if( mRunning ) {
		Stop();
	}

	mRunning = true;

    if( bFlag ) {
    	// Startialize SSL and BIO
    	mpSSL = SSL_new(gpSSLCtx);
    	SSL_set_ex_data(mpSSL, 0, this);
    	SSL_set_info_callback(mpSSL, DtlsSession::SSL_Info_Callback);
    	SSL_set_mtu(mpSSL, MTU);

    	mpReadBIO = BIO_new(BIO_s_mem());
    	BIO_set_mem_eof_return(mpReadBIO, -1);
    	mpWriteBIO = BIO_new(BIO_s_mem());
    	BIO_set_mem_eof_return(mpWriteBIO, -1);

//    	BIO_ctrl(mpReadBIO, BIO_CTRL_DGRAM_SET_MTU, MTU, NULL);
//    	BIO_ctrl(mpWriteBIO, BIO_CTRL_DGRAM_SET_MTU, MTU, NULL);

    	SSL_set_bio(mpSSL, mpReadBIO, mpWriteBIO);
    	if ( bActive ) {
    		SSL_set_connect_state(mpSSL);
    	} else {
    		SSL_set_accept_state(mpSSL);
    	}
    	EC_KEY *ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    	SSL_set_options(mpSSL, SSL_OP_SINGLE_ECDH_USE);
    	SSL_set_tmp_ecdh(mpSSL, ecdh);
    	EC_KEY_free(ecdh);
    }

	if( bFlag ) {
		LogAync(
				LOG_INFO,
				"DtlsSession::Start( "
				"this : %p, "
				"[OK] "
				")",
				this
				);
	} else {
		LogAync(
				LOG_ALERT,
				"DtlsSession::Start( "
				"this : %p, "
				"[Fail] "
				")",
				this
				);
		Stop();
	}

	mClientMutex.unlock();

	return bFlag;
}

void DtlsSession::Stop() {
	mClientMutex.lock();
	if( mRunning ) {
		mRunning = false;

		LogAync(
				LOG_INFO,
				"DtlsSession::Stop( "
				"this : %p "
				")",
				this
				);

	    if ( mpSSL ) {
	    	// SSL_free will free all BIO already set
	        SSL_free(mpSSL);
	        mpSSL = NULL;
	        mpReadBIO = NULL;
	        mpWriteBIO = NULL;
	    } else {
		    if( mpReadBIO ) {
		    	BIO_free(mpReadBIO);
		    	mpReadBIO = NULL;
		    }

		    if( mpWriteBIO ) {
		    	BIO_free(mpWriteBIO);
		    	mpWriteBIO = NULL;
		    }
	    }

		memset(mClientSalt, 0, sizeof(mClientSalt));
		memset(mServerKey, 0, sizeof(mServerKey));

		mDtlsSessionStatus = DtlsSessionStatus_None;

		LogAync(
				LOG_INFO,
				"DtlsSession::Stop( "
				"this : %p, "
				"[OK] "
				")",
				this
				);
	}
	mClientMutex.unlock();
}

DtlsSessionStatus DtlsSession::GetDtlsSessionStatus() {
	return mDtlsSessionStatus;
}

bool DtlsSession::GetClientKey(char *key, int& len) {
	bool bFlag = false;
	if( mDtlsSessionStatus == DtlsSessionStatus_HandshakeDone ) {
        memcpy(key, mClientSalt, sizeof(mClientSalt));
        len = sizeof(mClientSalt);
        bFlag = true;
	}
	return bFlag;
}

bool DtlsSession::GetServerKey(char *key, int& len) {
	bool bFlag = false;
	if( mDtlsSessionStatus == DtlsSessionStatus_HandshakeDone ) {
        memcpy(key, mServerKey, sizeof(mServerKey));
        len = sizeof(mServerKey);
        bFlag = true;
	}
	return bFlag;
}

bool DtlsSession::Handshake() {
	bool bFlag = true;

	LogAync(
			LOG_INFO,
			"DtlsSession::Handshake( "
			"this : %p "
			")",
			this
			);

	int ret;
	mClientMutex.lock();
	if ( mpSSL ) {
		ret = SSL_do_handshake(mpSSL);
	}
	mClientMutex.unlock();

	bFlag = FlushSSL();

	return bFlag;
}

bool DtlsSession::RecvFrame(const char* frame, unsigned int size) {
	bool bFlag = false;

	if( IsDTLS(frame, size) && (mDtlsSessionStatus != DtlsSessionStatus_HandshakeDone) ) {
		mClientMutex.lock();
		int written = BIO_write(mpReadBIO, frame, size);

		LogAync(
				LOG_DEBUG,
				"DtlsSession::RecvFrame( "
				"this : %p, "
				"written : %d "
				")",
				this,
				written
				);
	    char data[1500];
	    memset(&data, 0, 1500);
		int read = SSL_read(mpSSL, &data, 1500);
		if ( read < 0 ) {
		    unsigned long ret = SSL_get_error(mpSSL, read);
		    if ( ret == SSL_ERROR_SSL ) {
		        char error_string[200];
		        ERR_error_string_n(ERR_get_error(), error_string, 200);
		    	LogAync(
		    			LOG_WARNING,
		    			"DtlsSession::RecvFrame( "
		    			"this : %p, "
						"[DTLS Handshake Error], "
						"error : %d, "
		    			"error_string : %s "
		    			")",
		    			this,
						ERR_get_error(),
						error_string
		    			);
		    }
		}
		mClientMutex.unlock();

		bFlag = FlushSSL();
	} else {
		bFlag = false;
	}

	return bFlag;
}

bool DtlsSession::FlushSSL() {
	bool bFlag = true;

	mClientMutex.lock();
	int pending = BIO_ctrl_pending(mpWriteBIO);
	mClientMutex.unlock();

	LogAync(
			LOG_DEBUG,
			"DtlsSession::FlushSSL( "
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
		mClientMutex.lock();
		int pktSize = BIO_read(mpWriteBIO, dataBuffer, dataSize);
		mClientMutex.unlock();
		LogAync(
				LOG_DEBUG,
				"DtlsSession::FlushSSL( "
				"this : %p, "
				"sent : %d "
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

    	mClientMutex.lock();
		/* Check if there's anything left to send (e.g., fragmented packets) */
		pending = BIO_ctrl_pending(mpWriteBIO);
		mClientMutex.unlock();
	}

	if( bFlag ) {
		CheckHandshake();
	}

	return bFlag;
}

void DtlsSession::CheckHandshake() {
	mClientMutex.lock();
	if ( (mDtlsSessionStatus == DtlsSessionStatus_HandshakeDone) && SSL_is_init_finished(mpSSL) ) {
		// Check if peer send certificate
		X509 *cert = SSL_get_peer_certificate(mpSSL);
		unsigned char remoteFingerprint[EVP_MAX_MD_SIZE * 3];
		memset(remoteFingerprint, 0, sizeof(remoteFingerprint));
	    unsigned int fingerprintSize;
	    unsigned char fingerprint[EVP_MAX_MD_SIZE];
	    bool bFlag = X509_digest(cert, EVP_sha256(), (unsigned char *)fingerprint, &fingerprintSize);
        X509_free(cert);

        if( bFlag ) {
			char *c = (char *)&remoteFingerprint;
			for(int i = 0; i < (int)fingerprintSize; i++) {
				sprintf(c, "%.2X:", fingerprint[i]);
				c += 3;
			}
			if( bFlag ) {
				*(c - 1) = 0;
			}
        }

	    const char *cipherName = SSL_get_cipher(mpSSL);
	    SRTP_PROTECTION_PROFILE *srtpProfile = SSL_get_selected_srtp_profile(mpSSL);
        unsigned char material[SRTP_MASTER_LENGTH * 2];
        unsigned char *clientKey, *clientSalt, *serverKey, *serverSalt;
        int ret = 0;

		// Export keying material for SRTP
		static const char *label = "EXTRACTOR-dtls_srtp";
		ret = SSL_export_keying_material(mpSSL, material, SRTP_MASTER_LENGTH * 2, label, strlen(label), NULL, 0, 0);
		bFlag = (ret == 1);

        if( bFlag ) {
        	// Key derivation (http://tools.ietf.org/html/rfc5764#section-4.2)
        	// Just for client
        	clientKey = material;
        	serverKey = clientKey + SRTP_MASTER_KEY_LENGTH;
            clientSalt = serverKey + SRTP_MASTER_KEY_LENGTH;
            serverSalt = clientSalt + SRTP_MASTER_SALT_LENGTH;

            memcpy(&mClientSalt[0], clientKey, SRTP_MASTER_KEY_LENGTH);
            memcpy(&mClientSalt[SRTP_MASTER_KEY_LENGTH], clientSalt, SRTP_MASTER_SALT_LENGTH);

            memcpy(&mServerKey[0], serverKey, SRTP_MASTER_KEY_LENGTH);
            memcpy(&mServerKey[SRTP_MASTER_KEY_LENGTH], serverSalt, SRTP_MASTER_SALT_LENGTH);

			LogAync(
					LOG_INFO,
					"DtlsSession::CheckHandshake( "
					"this : %p, "
					"[DTLS Handshake OK], "
					"remoteFingerprint : %s, "
					"cipherName : %s, "
					"srtpProfile : %s "
					")",
					this,
					remoteFingerprint,
					cipherName,
					srtpProfile->name
					);
        } else {
			LogAync(
					LOG_WARNING,
					"DtlsSession::CheckHandshake( "
					"this : %p, "
					"[DTLS Handshake Fail], "
					"remoteFingerprint : %s, "
					"cipherName : %s, "
					"srtpProfile : %s "
					"ret : %d "
					")",
					this,
					remoteFingerprint,
					cipherName,
					srtpProfile->name,
					ret
					);
        }
	}
	mClientMutex.unlock();
}

bool DtlsSession::IsDTLS(const char *frame, unsigned len) {
	bool bFlag = false;
	if( len > 0 ) {
		// [20,64]
		bFlag = ((frame[0] >= 0x14) && (frame[0] <= 0x40));
	}
	return bFlag;
}

} /* namespace mediaserver */
