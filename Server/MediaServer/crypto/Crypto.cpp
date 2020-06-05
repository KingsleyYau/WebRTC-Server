/*
 * Crypto.cpp
 *
 *  Created on: 2019/12/26
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "Crypto.h"

#include "string.h"

namespace mediaserver {

Crypto::Crypto() {
	// TODO Auto-generated constructor stub
}

Crypto::~Crypto() {
	// TODO Auto-generated destructor stub
}

int Crypto::Sha1(const string& key, const string& data, unsigned char* result) {
    char digest[EVP_MAX_MD_SIZE + 1] = {0};
    unsigned int digestLen = 0;

    // Using sha1 hash engine here.
    // You may use other hash engines. e.g EVP_md5(), EVP_sha224, EVP_sha512, etc
    HMAC(EVP_sha1(), (const void *)key.c_str(), (int)key.length(), (const unsigned char *)data.c_str(), (int)data.length(), (unsigned char*)digest, &digestLen);

    if ( digestLen ) {
    	memcpy(result, digest, digestLen);
    	result[digestLen] = 0;
    }

	return digestLen;
}

} /* namespace mediaserver */
