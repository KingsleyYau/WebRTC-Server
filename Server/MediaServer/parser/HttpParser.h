/*
 * HttpParser.h
 *
 *  Created on: 2015-9-28
 *      Author: Max
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef PARSER_HTTPPARSER_H_
#define PARSER_HTTPPARSER_H_

#include "DataParser.h"

#include <common/KMutex.h>
#include <common/Arithmetic.h>

#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>
#include <algorithm>

#include <string>
#include <map>
using namespace std;

namespace mediaserver {

typedef map<string, string> Parameters;

typedef enum HttpState {
	HttpState_UnKnow = 0,
	HttpState_Header,
	HttpState_Body,
} HttpState;

typedef enum HttpType {
	GET,
	POST,
	UNKNOW,
} HttpType;

class HttpParser;
class HttpParserCallback {
public:
	virtual ~HttpParserCallback(){};
	virtual void OnHttpParserHeader(HttpParser* parser) = 0;
	virtual void OnHttpParserBody(HttpParser* parser) = 0;
	virtual void OnHttpParserError(HttpParser* parser) = 0;
};

class HttpParser : public DataParser {
public:
	HttpParser();
	virtual ~HttpParser();

	void SetCallback(HttpParserCallback* callback);

	int ParseData(char* buffer, int len);

	string GetRawFirstLine();
	string GetAuth();
	string GetParam(const string& key);
	string GetPath();
	HttpType GetType();

	const char* GetBody();

private:
	HttpType mHttpType;
	string mPath;
	int miContentLength;
	Parameters mParameters;
	char* mpBody;
	int miCurContentIndex;
	string mContentType = "";
	string mAuth = "";
	string mRawFirstLine = "";

	KMutex mClientMutex;
	HttpState mState;
	HttpParserCallback* mpCallback;

	bool ParseFirstLine(const string& line);
	void ParseParameters(const string& line);
	void ParseHeader(const string& line);

	void Lock();
	void Unlock();
};
}
#endif /* PARSER_HTTPPARSER_H_ */
