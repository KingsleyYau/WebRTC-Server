/*
 * Crypto.h
 *
 *  Created on: 2019/12/26
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef CRYPTO_CRYPTO_H_
#define CRYPTO_CRYPTO_H_

#include <unistd.h>

#include <string>
using namespace std;

#include <openssl/hmac.h>

namespace mediaserver {

class Crypto {
public:
	Crypto();
	virtual ~Crypto();

public:
	static int Sha1(const string& key, const string& data, unsigned char* result);
};

} /* namespace mediaserver */

#endif /* CRYPTO_CRYPTO_H_ */
