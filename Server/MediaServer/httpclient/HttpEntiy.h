/*
 * HttpEntiy.h
 *
 *  Created on: 2014-12-24
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef HTTPCLIENT_HTTPENTIY_
#define HTTPCLIENT_HTTPENTIY_

#include <string>
#include <map>
using namespace std;

typedef map<string, string> HttpMap;

typedef struct _tagFileMapItem
{
	string	fileName;
	string	mimeType;
} FileMapItem;
typedef map<string, FileMapItem> FileMap;

class HttpEntiy {
	friend class HttpClient;

public:
	HttpEntiy();
	virtual ~HttpEntiy();

	void AddHeader(string key, string value);
	void SetKeepAlive(bool isKeepAlive);
	void SetAuthorization(string user, string password);
	void SetGetMethod(bool isGetMethod);
	void SetSaveCookie(bool isSaveCookie);
	void SetRawData(string data);
	void AddContent(string key, string value);
	void AddFile(string key, string fileName, string mimeType = "image/jpeg");

	void Reset();

	/**
	 * 获取请求参数
	 * @return 请求参数文本输出
	 */
    string GetContentDesc();

private:
	HttpMap mHeaderMap;
	HttpMap mContentMap;
	FileMap mFileMap;
	string mAuthorization;
	string mRawData;
	bool mIsGetMethod;
	bool mbSaveCookie;
};

#endif /* HTTPCLIENT_HTTPENTIY_ */
