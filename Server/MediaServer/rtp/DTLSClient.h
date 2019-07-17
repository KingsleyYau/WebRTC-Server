/*
 * DTLSClient.h
 *
 *  Created on: 2019/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RTP_DTLSCLIENT_H_
#define RTP_DTLSCLIENT_H_

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

class DTLSClient {
public:
	DTLSClient();
	virtual ~DTLSClient();

public:
	static bool GobalInit();
	static bool IsDTLS(const char *frame, unsigned len);

public:
	void SetSocketSender(SocketSender *sender);
	bool Start();
	void Stop();

	bool Handshake();

	bool RecvFrame(const char* frame, unsigned int size);

	bool IsHandshakeFinish();
	bool GetClientKey(char *key, int& len);
	bool GetServerKey(char *key, int& len);

private:
	static int SSL_Generate_Keys(X509** certificate, EVP_PKEY** privateKey);
	static bool SSL_Load_Keys(const char* server_pem, const char* server_key, X509** certificate, EVP_PKEY** privateKey);
	static void SSL_Info_Callback(const SSL* s, int where, int ret);

private:
	bool CheckHandshake();

private:
	// Status
	KMutex mClientMutex;
	bool mRunning;
	bool mHandshakeFinish;

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

#endif /* RTP_DTLSCLIENT_H_ */
