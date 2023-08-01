/*
 * HttpClient.h
 *
 *  Created on: 2014-12-24
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef HTTPCLIENT_HTTPCLIENT_H_
#define HTTPCLIENT_HTTPCLIENT_H_

#include "HttpEntiy.h"
#include <curl/curl.h>

#include <list>
#include <string>
using namespace std;

class HttpClient {
public:
	HttpClient();
	virtual ~HttpClient();

	/**
	 * 初始化
	 */
	static void Init();

	/**
	 * 停止请求, 可多线程调用
	 */
	void Stop();

	/**
	 * 开始请求
	 */
	bool Request(const string& url, const HttpEntiy* entiy, bool closeAfterRequest = true);

	/**
	 * 获取返回值
	 */
	long GetRespondCode();
	/**
	 * 获取返回头类型
	 */
	string GetContentType() const;
	/**
	 * 获取返回内容
	 */
	void GetBody(const char** pBuffer, int& size);
	/**
	 * 获取最后一次错误描述
	 */
	const char* GetLastError();

	/**
	 * 清除所有域名cookies
	 */
	static void CleanCookies();
	/**
	 * 获取指定域名cookies
	 */
	static string GetCookies(string site);
	/**
	 * 获取所有域名的cookies
	 */
	static list<string> GetCookiesInfo();
	/**
	 * 设置cookies
	 */
	static void SetCookiesInfo(const list<string>& cookies);

private:
	void InitRespondBuffer();
	void DestroyBuffer();
	void ResetBuffer();
	bool AddRespondBuffer(const char* buf, int size);
	void Close();

	static CURLcode Curl_SSL_Handle(CURL *curl, void *sslctx, void *param);
	static size_t CurlHandle(void *buffer, size_t size, size_t nmemb, void *data);
	void HttpHandle(void *buffer, size_t size, size_t nmemb);

	static size_t CurlProgress(
			void *data,
            double downloadTotal,
            double downloadNow,
            double uploadTotal,
            double uploadNow
            );
	size_t HttpProgress(
            double downloadTotal,
            double downloadNow,
            double uploadTotal,
            double uploadNow
            );

	CURL *mpCURL;
	string mUrl;
	string mContentType;
	double mContentLength;
	long mHttpCode;
	CURLcode mLastRes;

	// stop manually
	bool mbStop;

	// application timeout
	double mdUploadTotal;
	double mdUploadLast;
	double mdUploadLastTime;
	double mdDownloadTotal;
	double mdDownloadLast;
	double mdDownloadLastTime;

	// buffer
	char* mpRespondBuffer;
	int miCurrentSize;
	int miCurrentCapacity;
};

#endif /* HTTPCLIENT_HTTPCLIENT_H_ */
