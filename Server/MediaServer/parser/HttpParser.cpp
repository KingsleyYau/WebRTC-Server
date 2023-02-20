/*
 * HttpParser.cpp
 *
 *  Created on: 2015-9-28
 *      Author: Max
 *      Email: Kingsleyyau@gmail.com
 */

#include "HttpParser.h"

#include <common/Math.h>
#include <common/Arithmetic.h>
#include <common/StringHandle.h>
#include <common/LogManager.h>

namespace mediaserver {
#define HTTP_URL_MAX_FIRST_LINE 2048
#define HTTP_URL_MAX_PATH 4096
#define HTTP_MAX_PARSER_BUFFER_WITHOUT_CONTENT_LENGTH 4096

#define HTTP_PARAM_SEP ":"
#define HTTP_LINE_SEP "\r\n"
#define HTTP_HEADER_SEP "\r\n\r\n"

#define HTTP_HEADER_CONTENTLENGTH "Content-Length"
#define HTTP_HEADER_CONTENTTYPE "Content-Type"
#define HTTP_HEADER_CONTENTTYPE_URLENCODED "application/x-www-form-urlencoded"
#define HTTP_HEADER_AUTH "Authorization"

HttpParser::HttpParser():mClientMutex(KMutex::MutexType_Recursive) {
	// TODO Auto-generated constructor stub
	mpBody = NULL;
	Reset();
}

HttpParser::~HttpParser() {
	// TODO Auto-generated destructor stub
}

void HttpParser::SetCallback(HttpParserCallback* callback) {
	mpCallback = callback;
}

int HttpParser::ParseData(char* buffer, int len) {
	int ret = 0;
	int last = len;

	Lock();
	mbContinue = true;
	while( mbContinue ) {
		switch( mState ) {
		case HttpState_UnKnow : {
			int lineNumber = 0;
			bool bFlag = false;

			char line[HTTP_URL_MAX_PATH];
			int lineLen = 0;

			const char* header = buffer;
			const char* sepHeader = strstr(buffer, HTTP_HEADER_SEP);
			const char* sep = NULL;
			if (sepHeader) {
				// 接收头部完成
				mState = HttpState_Header;

				// Parse HTTP header separator
				ret += sepHeader - buffer + strlen(HTTP_HEADER_SEP);

				// Parse HTTP header line separator
				while (true) {
					if ((sep = strstr(header, HTTP_LINE_SEP)) && (sep != (sepHeader + 2))) {
						lineLen = sep - header;
						if( lineLen < (int)(sizeof(line) - 1) ) {
							memcpy(line, header, lineLen);
							line[lineLen] = '\0';

							if( lineNumber == 0 ) {
								// 暂时只获取第一行
								mRawFirstLine = line;
								bFlag = ParseFirstLine(line);
							} else {
								ParseHeader(line);
							}
						}

						// 换行
						header += lineLen + strlen(HTTP_LINE_SEP);
						lineNumber++;

					} else {
						break;
					}
				}
			}

			if (mState == HttpState_Header) {
				// 解析头部完成
				last = len - ret;
				if (bFlag) {
					// 解析第一行完成
					if( mpCallback ) {
						mpCallback->OnHttpParserHeader(this);
					}
					// 没有Content-Length, 重置解析
					if (miContentLength == 0) {
						Reset();
					}
				} else {
					// 解析第一行错误
					if( mpCallback ) {
						mpCallback->OnHttpParserError(this);
					}
					Reset();
				}
			} else if (len > HTTP_URL_MAX_PATH) {
				// 头部超过限制 HTTP_URL_MAX_PATH
				if( mpCallback ) {
					mpCallback->OnHttpParserError(this);
				}
				Reset();
			} else {
				// 数据不够, 继续收数据
				mbContinue = false;
			}
		}break;
		case HttpState_Header:{
			mbContinue = false;
			// 接收头部完成, 继续接收body
			if (miContentLength > 0) {
				if (miCurContentIndex == 0) {
					if (mpBody) {
						delete[] mpBody;
						mpBody = NULL;
					}
					mpBody = new char[miContentLength + 1];
					memset(mpBody, '\0', miContentLength + 1);
				}

				int readLength = MIN(last, miContentLength - miCurContentIndex);
				if (readLength > 0) {
					memcpy(mpBody + miCurContentIndex, buffer + ret, readLength);
					miCurContentIndex += readLength;
					ret += readLength;
					last -= readLength;

					if( miCurContentIndex >= miContentLength ) {
						// 接收Body完成
						mState = HttpState_Body;

						if( mpCallback ) {
							mpCallback->OnHttpParserBody(this);
						}

						Reset();
					}
				}

			} else {
				// 没有Content-Length的请求不解析, 并且数据过大
//				if (ret > HTTP_MAX_PARSER_BUFFER_WITHOUT_CONTENT_LENGTH) {
//					if( mpCallback ) {
//						mpCallback->OnHttpParserError(this);
//					}
//				}
//				ret += last;
				// 没有Content-Length的请求, 并且不是Get
				if (mHttpType != GET) {
					if( mpCallback ) {
						mpCallback->OnHttpParserError(this);
					}
				}
				Reset();
			}
		}break;
		case HttpState_Body:{
			mbContinue = false;
			// 接收body完成还有数据, 出错
			if( mpCallback ) {
				mpCallback->OnHttpParserError(this);
			}
		}break;
		default:{
			mbContinue = false;
			break;
		}
		}
	}
	Unlock();

	return ret;
}

void HttpParser::Reset() {
	Lock();
	mHttpType = UNKNOW;
	mRawFirstLine = "";
	mPath = "";
	miContentLength = 0;
	miCurContentIndex = 0;
	mContentType = "";
	mState = HttpState_UnKnow;
	mbContinue = false;

	if( mpBody != NULL ) {
		delete[] mpBody;
		mpBody = NULL;
	}
	Unlock();

	DataParser::Reset();
}

string HttpParser::GetParam(const string& key)  {
	string result = "";
	Parameters::iterator itr = mParameters.find(key.c_str());
	if( itr != mParameters.end() ) {
		result = (itr->second);
	}
	return result;
}

string HttpParser::GetRawFirstLine() {
	return mRawFirstLine;
}

string HttpParser::GetPath() {
	return mPath;
}

HttpType HttpParser::GetType() {
	return mHttpType;
}

int HttpParser::GetContentLength() {
	return miContentLength;
}

string HttpParser::GetAuth() {
	return mAuth;
}

const char* HttpParser::GetBody() {
	return mpBody;
}

bool HttpParser::ParseFirstLine(const string& wholeLine) {
	bool bFlag = true;
	string line;
	int i = 0;
	string::size_type index = 0;
	string::size_type pos;

	while( string::npos != index ) {
		pos = wholeLine.find(" ", index);
		if( string::npos != pos ) {
			// 找到分隔符
			line = wholeLine.substr(index, pos - index);
			// 移动下标
			index = pos + 1;

		} else {
			// 是否最后一次
			if( index < wholeLine.length() ) {
				line = wholeLine.substr(index, pos - index);
				// 移动下标
				index = string::npos;

			} else {
				// 找不到
				index = string::npos;
				break;
			}
		}

		switch(i) {
		case 0:{
			// 解析http type
			if( strcasecmp("GET", line.c_str()) == 0 ) {
				mHttpType = GET;

			} else if( strcasecmp("POST", line.c_str()) == 0 ) {
				mHttpType = POST;

			} else {
				bFlag = false;
				break;
			}
		}break;
		case 1:{
			// 解析url
//			Arithmetic ari;
//			char temp[HTTP_URL_MAX_FIRST_LINE] = {'\0'};
//			int decodeLen = ari.decode_url(line.c_str(), line.length(), temp);
//			temp[decodeLen] = '\0';
			string path = line;
			string::size_type posSep = path.find("?");
			if( (string::npos != posSep) && (posSep + 1 < path.length()) ) {
				// 解析路径
				mPath = path.substr(0, posSep);
				// 解析参数
				string param = path.substr(posSep + 1, path.length() - (posSep + 1));
				ParseParameters(param);

			} else {
				mPath = path;
			}

		}break;
		default:break;
		};

		i++;
	}

	return bFlag;
}

void HttpParser::ParseParameters(const string& wholeLine) {
	string key;
	string value;

	string line;
	string::size_type posSep;
	string::size_type index = 0;
	string::size_type pos;

	while( string::npos != index ) {
		pos = wholeLine.find("&", index);
		if( string::npos != pos ) {
			// 找到分隔符
			line = wholeLine.substr(index, pos - index);
			// 移动下标
			index = pos + 1;

		} else {
			// 是否最后一次
			if( index < wholeLine.length() ) {
				line = wholeLine.substr(index, pos - index);
				// 移动下标
				index = string::npos;

			} else {
				// 找不到
				index = string::npos;
				break;
			}
		}

		posSep = line.find("=");
		if( (string::npos != posSep) && (posSep + 1 < line.length()) ) {
			key = line.substr(0, posSep);
			value = line.substr(posSep + 1, line.length() - (posSep + 1));

			Arithmetic arc;
			char tmp[4096];
			if ( value.length() < sizeof(tmp) ) {
				arc.decode_url(value.c_str(), value.length(), tmp);
				mParameters.insert(Parameters::value_type(key, tmp));
			}

		} else {
			key = line;
		}

	}
}

void HttpParser::ParseHeader(const string& line) {
	string key = "";
	string value = "";

	// Parse HTTP header line parameter separator
	string::size_type pos = line.find(HTTP_PARAM_SEP, 0);
	if( string::npos != pos ) {
		// 找到分隔符
		key = line.substr(0, pos);
		value = StringHandle::trim(line.substr(pos + 1, line.size() - 1));
	}

	if ( strcasecmp(key.c_str(), HTTP_HEADER_CONTENTLENGTH) == 0 ) {
		miContentLength = atoi(value.c_str());
	} else if ( strcasecmp(key.c_str(), HTTP_HEADER_CONTENTTYPE) == 0 ) {
		mContentType = value;
	} else if ( strcasecmp(key.c_str(), HTTP_HEADER_AUTH) == 0 ) {
		string::size_type posAuth = value.find(" ", 0);
		string auth = StringHandle::trim(value.substr(posAuth + 1, value.size() - 1));
		mAuth = auth;
	}
}

void HttpParser::Lock() {
	mClientMutex.lock();
}

void HttpParser::Unlock() {
	mClientMutex.unlock();
}
}
