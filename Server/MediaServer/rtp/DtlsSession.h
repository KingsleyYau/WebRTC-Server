/*
 * DtlsSession.h
 * DTLS会话管理器
 *  Created on: 2019/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_DTLSSESSION_H_
#define RTP_DTLSSESSION_H_

#include <common/KMutex.h>

#include <socket/ISocketSender.h>

// libopenssl
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/asn1.h>
#include <openssl/bio.h>

#include <string>
using namespace std;

/* SRTP stuff (http://tools.ietf.org/html/rfc3711) */
#define SRTP_MASTER_KEY_LENGTH 16
#define SRTP_MASTER_SALT_LENGTH	14
#define SRTP_MASTER_LENGTH (SRTP_MASTER_KEY_LENGTH + SRTP_MASTER_SALT_LENGTH)

namespace mediaserver {
typedef enum DtlsSessionStatus {
	DtlsSessionStatus_None,
	DtlsSessionStatus_HandshakeStart,
	DtlsSessionStatus_HandshakeDone,
	DtlsSessionStatus_Alert
} DtlsSessionStatus;

class DtlsSession {
public:
	DtlsSession();
	virtual ~DtlsSession();

public:
	static bool GobalInit(const string& certPath, const string& keyPath);
	static const unsigned char *GetFingerprint();
	static bool IsDTLS(const char *frame, unsigned len);

public:
	void SetSocketSender(SocketSender *sender);
	bool Start(bool bActive = true);
	void Stop();

	bool Handshake();

	bool RecvFrame(const char* frame, unsigned int size);

	DtlsSessionStatus GetDtlsSessionStatus();
	bool GetClientKey(char *key, int& len);
	bool GetServerKey(char *key, int& len);

private:
	static int SSL_Generate_Keys(X509** certificate, EVP_PKEY** privateKey);
	static bool SSL_Load_Keys(const char* server_pem, const char* server_key, X509** certificate, EVP_PKEY** privateKey);
	static void SSL_Info_Callback(const SSL* s, int where, int ret);

private:
	bool FlushSSL();
	void CheckHandshake();

private:
	// Status
	KMutex mClientMutex;
	bool mRunning;
	DtlsSessionStatus mDtlsSessionStatus;

	// Socket
	SocketSender *mpSocketSender;

    // SSL
    SSL *mpSSL;
    // Read BIO (incoming DTLS data)
    BIO *mpReadBIO;
    // Write BIO (outgoing DTLS data)
    BIO *mpWriteBIO;

    // DTSL
    char mClientSalt[SRTP_MASTER_LENGTH];
    char mServerKey[SRTP_MASTER_LENGTH];
};

} /* namespace mediaserver */

#endif /* RTP_DTLSSESSION_H_ */
