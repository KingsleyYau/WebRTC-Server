/*
 * HttpClient.cpp
 *
 *  Created on: 2014-12-24
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include "HttpClient.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <common/KLog.h>

#define USER_AGENT	"Mozil1a/4.0 (compatible; MS1E 7.0; Windows NT 6.1; WOW64;)"
#define MAX_RESPONED_BUFFER 4096
#define DWONLOAD_TIMEOUT 30

CURLSH *sh;

void HttpClient::Init() {
	curl_global_init(CURL_GLOBAL_ALL);
	curl_version_info_data *data = curl_version_info(CURLVERSION_FIRST);

	if (data->version != NULL) {
		FileLog("httpclient", "Init, curl_version:%s", data->version);
	}

	if (data->ssl_version != NULL) {
		FileLog("httpclient", "Init, ssl_version:%s", data->ssl_version);
	}

	sh = curl_share_init();
	curl_share_setopt(sh, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
}

HttpClient::HttpClient() {
	// TODO Auto-generated constructor stub
	mpCURL = NULL;
	mLastRes = CURLE_OK;
	mUrl = "";

	mbStop = false;

	mHttpCode = 0;
	mContentType = "";
	mContentLength = -1;

	mdUploadTotal = -1;
	mdUploadLast = -1;
	mdUploadLastTime = -1;
	mdDownloadTotal = -1;
	mdDownloadLast = -1;
	mdDownloadLastTime = -1;

	mpRespondBuffer = NULL;
	miCurrentSize = 0;
	miCurrentCapacity = 0;

	InitRespondBuffer();
}

HttpClient::~HttpClient() {
	// TODO Auto-generated destructor stub
	Close();
	DestroyBuffer();
}

void HttpClient::Stop() {
	FileLog("httpclient", "HttpClient::Stop");
	mbStop = true;
}

void HttpClient::Close() {
	FileLog("httpclient", "HttpClient::Close");
	if (mpCURL != NULL) {
		curl_easy_cleanup(mpCURL);
		mpCURL = NULL;
	}
}

bool HttpClient::Request( const string& url, const HttpEntiy* entiy, bool closeAfterRequest) {
	FileLog("httpclient", "HttpClient::Request, url:%s, entiy:%p",
			url.c_str(), entiy);

	bool bFlag = true;

	mUrl = url;
	mbStop = false;

	mHttpCode = 0;
	mContentType = "";
	mContentLength = -1;

	mdUploadTotal = -1;
	mdUploadLast = -1;
	mdUploadLastTime = -1;
	mdDownloadTotal = -1;
	mdDownloadLast = -1;
	mdDownloadLastTime = -1;

	ResetBuffer();

	if (mpCURL == NULL) {
		mpCURL = curl_easy_init();
	}

	curl_easy_setopt(mpCURL, CURLOPT_URL, mUrl.c_str());
	curl_easy_setopt(mpCURL, CURLOPT_SHARE, sh);

    // 支持重定向
    curl_easy_setopt(mpCURL, CURLOPT_FOLLOWLOCATION, 1L);
    
	// 处理http body函数
	curl_easy_setopt(mpCURL, CURLOPT_WRITEFUNCTION, CurlHandle);
	curl_easy_setopt(mpCURL, CURLOPT_WRITEDATA, this);

	// deal with progress
	curl_easy_setopt(mpCURL, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(mpCURL, CURLOPT_PROGRESSFUNCTION, CurlProgress);
	curl_easy_setopt(mpCURL, CURLOPT_PROGRESSDATA, this);

	// tcp keep alive
	curl_easy_setopt(mpCURL, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(mpCURL, CURLOPT_TCP_KEEPIDLE, 10L);
	curl_easy_setopt(mpCURL, CURLOPT_TCP_KEEPINTVL, 10L);

	string host = mUrl;
	std::size_t index = mUrl.find("http://");
	if (index != string::npos) {
		host = host.substr(strlen("http://"), host.length() - strlen("http://"));
		index = host.find("/");
		if (index != string::npos) {
			host = host.substr(0, index);
		}
	}

	string cookie = GetCookies(host);
	FileLog("httpclient", "HttpClient::Request, Cookie Send:%s", cookie.c_str());

	//	curl_easy_setopt(mpCURL, CURLOPT_FOLLOWLOCATION, 1);
	// 设置连接超时
	curl_easy_setopt(mpCURL, CURLOPT_CONNECTTIMEOUT, 20L);
	// 设置不抛退出信号量
	curl_easy_setopt(mpCURL, CURLOPT_NOSIGNAL, 1L);
	// 设置User-Agent
	curl_easy_setopt(mpCURL, CURLOPT_USERAGENT, USER_AGENT);

	if (entiy != NULL) {
		if (!entiy->mIsGetMethod) {
			curl_easy_setopt(mpCURL, CURLOPT_POST, 1);
		}
	}

	// 处理https
	if (mUrl.find("https") != string::npos) {
		FileLog("httpclient", "HttpClient::Request, Connect with SSL");
		// 不检查服务器证书
		curl_easy_setopt(mpCURL, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(mpCURL, CURLOPT_SSL_VERIFYHOST, 0);
		// 不设置客户端证书和私钥
//		curl_easy_setopt(mpCURL, CURLOPT_SSLCERTTYPE, "PEM");
//		curl_easy_setopt(mpCURL , CURLOPT_SSL_CTX_FUNCTION, Curl_SSL_Handle);
//		curl_easy_setopt(mpCURL , CURLOPT_SSL_CTX_DATA, this);
	}

	struct curl_slist* pList = NULL;
	struct curl_httppost* pPost = NULL;
	struct curl_httppost* pLast = NULL;
	string postData("");

	if (entiy != NULL) {
		/* Basic Authentication */
		if (entiy->mAuthorization.length() > 0) {
			curl_easy_setopt(mpCURL, CURLOPT_USERPWD, entiy->mAuthorization.c_str());
			FileLog("httpclient", "HttpClient::Request, Add authentication header:[%s]", entiy->mAuthorization.c_str());
		}

		/* Headers */
		for (HttpMap::const_iterator itr = entiy->mHeaderMap.begin(); itr != entiy->mHeaderMap.end(); itr++) {
			string header = itr->first + ": " + itr->second;
			pList = curl_slist_append(pList, header.c_str());
			FileLog("httpclient", "HttpClient::Request, Add header:[%s]", header.c_str());
		}

        if (entiy->mRawData.length() > 0 && !entiy->mIsGetMethod) {
            curl_easy_setopt(mpCURL, CURLOPT_POSTFIELDS, entiy->mRawData.c_str());
        } else if (entiy->mFileMap.empty() && !entiy->mContentMap.empty() && !entiy->mIsGetMethod) {
            /* Contents */
            for (HttpMap::const_iterator itr = entiy->mContentMap.begin(); itr != entiy->mContentMap.end(); itr++) {
                if (!postData.empty()) {
                    postData += "&";
                }
                char* tempBuffer = curl_easy_escape(mpCURL, itr->second.c_str(), itr->second.length());
                if (NULL != tempBuffer) {
                    postData += itr->first;
                    postData += "=";
                    postData += tempBuffer;

                    curl_free(tempBuffer);
                }

                FileLog("httpclient", "HttpClient::Request, this:%p, Add content:[%s:%s]", this, itr->first.c_str(), itr->second.c_str());
            }

            curl_easy_setopt(mpCURL, CURLOPT_POSTFIELDS, postData.c_str());
        } else {

    		/* Contents */
    		for (HttpMap::const_iterator itr = entiy->mContentMap.begin(); itr != entiy->mContentMap.end(); itr++) {
    			curl_formadd(&pPost, &pLast, CURLFORM_COPYNAME, itr->first.c_str(),
    					CURLFORM_COPYCONTENTS, itr->second.c_str(), CURLFORM_END);
    			FileLog("httpclient", "HttpClient::Request, Add content:[%s:%s]", itr->first.c_str(), itr->second.c_str());
    		}

    		/* Files */
    		for (FileMap::const_iterator itr = entiy->mFileMap.begin(); itr != entiy->mFileMap.end(); itr++) {
    			curl_formadd(&pPost, &pLast, CURLFORM_COPYNAME, "filename",
    					CURLFORM_COPYCONTENTS, itr->first.c_str(), CURLFORM_END);

    			curl_formadd(&pPost, &pLast,
    					CURLFORM_COPYNAME, itr->first.c_str(),
    					CURLFORM_FILE, itr->second.fileName.c_str(),
    					CURLFORM_CONTENTTYPE, itr->second.mimeType.c_str(),
    					CURLFORM_END);

    			FileLog("httpclient", "HttpClient::Request, Add file filename:[%s], content [%s:%s,%s]"
    					, itr->first.c_str(), itr->first.c_str(), itr->second.fileName.c_str(), itr->second.mimeType.c_str());
    		}
        }

		if (pList != NULL) {
			curl_easy_setopt(mpCURL, CURLOPT_HTTPHEADER, pList);
		}
		if (pPost != NULL) {
			curl_easy_setopt(mpCURL, CURLOPT_HTTPPOST, pPost);
		}
	}

	FileLog("httpclient", "HttpClient::Request, curl_easy_perform");
	mLastRes = curl_easy_perform(mpCURL);

	double totalTime = 0;
	curl_easy_getinfo(mpCURL, CURLINFO_TOTAL_TIME, &totalTime);
	FileLog("httpclient", "HttpClient::Request, totalTime:%f second", totalTime);

	curl_easy_getinfo(mpCURL, CURLINFO_RESPONSE_CODE, &mHttpCode);
	FileLog("httpclient", "HttpClient::Request, mHttpCode:%ld", mHttpCode);

	if (closeAfterRequest) {
		Close();
	}

	if (pList != NULL) {
		curl_slist_free_all(pList);
	}
	if (pPost != NULL) {
		curl_formfree(pPost);
	}

	cookie = GetCookies(host);
	FileLog("httpclient", "HttpClient::Request, Cookie Recv:%s", cookie.c_str());

	bFlag = (mLastRes == CURLE_OK);
	FileLog("httpclient", "HttpClient::Request, bFlag:%s, res:%d", bFlag?"true":"false", mLastRes);

	return bFlag;
}

long HttpClient::GetRespondCode() {
	return mHttpCode;
}

string HttpClient::GetContentType() const
{
	return mContentType;
}

void HttpClient::GetBody(const char** pBuffer, int& size) {
	*pBuffer = mpRespondBuffer;
	size = miCurrentSize;
}

const char* HttpClient::GetLastError() {
	return curl_easy_strerror(mLastRes);
}

void HttpClient::CleanCookies() {
	FileLog("httpclient", "HttpClient::CleanCookies");
	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_SHARE, sh);
	curl_easy_setopt(curl, CURLOPT_COOKIELIST, "ALL");
	curl_easy_cleanup(curl);
}

list<string> HttpClient::GetCookiesInfo()
{
	list<string> cookiesInfo;

	CURL *curl = curl_easy_init();
	if (NULL != curl)
	{
		curl_easy_setopt(curl, CURLOPT_SHARE, sh);

		struct curl_slist *cookies = NULL;
		CURLcode res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
		if (CURLE_OK == res)
		{
			int i = 0;
			curl_slist* cookies_item = NULL;
			for (i = 0, cookies_item = cookies;
				cookies_item != NULL;
				i++, cookies_item = cookies_item->next)
			{
				if (NULL != cookies_item->data
					&& strlen(cookies_item->data) > 0)
				{
					FileLog("httpclient", "HttpClient::GetCookiesInfo, cookies_item->data:%s", cookies_item->data);

					cookiesInfo.push_back(cookies_item->data);
				}
			}
		}

		if (NULL != cookies)
		{
			curl_slist_free_all(cookies);
		}

		curl_easy_cleanup(curl);
	}

	return cookiesInfo;
}

void HttpClient::SetCookiesInfo(const list<string>& cookies)
{
	CURL *curl = curl_easy_init();
	if (NULL != curl)
	{
		curl_easy_setopt(curl, CURLOPT_SHARE, sh);

		string cookiesInfo("");
		for (list<string>::const_iterator iter = cookies.begin();
			iter != cookies.end();
			iter++)
		{
			curl_easy_setopt(curl, CURLOPT_COOKIELIST, (*iter).c_str());
		}

		curl_easy_cleanup(curl);
	}
}

string HttpClient::GetCookies(string site) {
	string cookie = "";
	FileLog("httpclient", "HttpClient::GetCookies, site:%s", site.c_str());

	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, site.c_str());
	curl_easy_setopt(curl, CURLOPT_SHARE, sh);

	CURLcode res;
	struct curl_slist *cookies = NULL;
	struct curl_slist *next = NULL;
	int i;

	res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
	if (res == CURLE_OK) {
		next = cookies, i = 0;
		while (next != NULL) {
			FileLog("httpclient", "HttpClient::GetCookies, cookies[%d]:%s", i++, next->data);

			/**
			 *
			 * www.example.com :	The domain that the cookie applies to
    		 * FALSE :				can other webservers within www.someURL.com access this cookie?
    		 * 						If there was a webserver at xxx.www.someURL.com, could it see this cookie?
    		 * 						If the domain begin with \u201c.\u201d, this value is TRUE
			 * / :					more restrictions on what paths on www.someURL.com can see this cookie.
			 * 						Anything here and under can see the cookie. / is the least restrictive,
			 * 						meaning that any (and all) requests from www.someURL.com get the cookie header sent.
    		 * FALSE :				Do we have to be coming in on https to send the cookie?
    		 * 0 :					expiration time. zero probably means that it never expires,
    		 * 						or that it is good for as long as this session lasts.
    		 * 						A number like 1420092061 would correspond to a number of seconds since the epoch (Jan 1, 1970).
    		 * 						If this value equal to 0, cookie don\u2019t expire
    		 * CNAME :				The name of the cookie variable that will be send to the server.
    		 * value :				cookie value that will be sent. Might be comma separated list of terms,
    		 * 						or could just be a word.. Hard to say. Depends on the server.
			 *
			 */
			int j = 0;
			bool bFlag = false;
			char *p = strtok(next->data, "\t");
			while(p != NULL) {
//				FileLog("httpclient", "HttpClient::GetCookies(p[%d]:%s)", j, p);
				switch(j) {
				case 0:{
					if (strcmp(p, site.c_str()) != 0) {
						// not current site
						bFlag = true;
					}
				}break;
				case 5:{
					cookie += p;
					cookie += "=";
				}break;
				case 6:{
					cookie += p;
					cookie += ";";
				}break;
				default:break;
				}

				if (bFlag) {
					break;
				}

				j++;
				p = strtok(NULL, "\t");
			}

			next = next->next;
		}

		if (cookies != NULL) {
			curl_slist_free_all(cookies);
		}
	}
	curl_easy_cleanup(curl);

	FileLog("httpclient", "HttpClient::GetCookies, cookie:%s", cookie.c_str());

	return cookie;
}

void HttpClient::InitRespondBuffer() {
	if (mpRespondBuffer != NULL) {
		delete[] mpRespondBuffer;
		mpRespondBuffer = NULL;
	}

	miCurrentSize = 0;
	miCurrentCapacity = MAX_RESPONED_BUFFER;
	mpRespondBuffer = new char[miCurrentCapacity];
}

void HttpClient::DestroyBuffer() {
	if (mpRespondBuffer) {
		delete[] mpRespondBuffer;
		mpRespondBuffer = NULL;
	}
}

void HttpClient::ResetBuffer() {
	if (mpRespondBuffer) {
		mpRespondBuffer[0] = '\0';
	}
	miCurrentSize = 0;
}

bool HttpClient::AddRespondBuffer(const char* buf, int size) {
	bool bFlag = false;
	if (size > 0) {
		/* Add buffer if buffer is not enough */
		while (size + miCurrentSize >= miCurrentCapacity) {
			miCurrentCapacity *= 2;
			bFlag = true;
		}
		if (bFlag) {
			char *newBuffer = new char[miCurrentCapacity];
			if (mpRespondBuffer != NULL) {
				memcpy(newBuffer, mpRespondBuffer, miCurrentSize);
				delete[] mpRespondBuffer;
				mpRespondBuffer = NULL;
			}
			mpRespondBuffer = newBuffer;
		}
		memcpy(mpRespondBuffer + miCurrentSize, buf, size);
		miCurrentSize += size;
		mpRespondBuffer[miCurrentSize] = '\0';
	}
	return true;
}

void HttpClient::HttpHandle(void *buffer, size_t size, size_t nmemb) {
	FileLog("httpclient", "HttpClient::HttpHandle, size:%d , nmemb:%d", size, nmemb);
	if (mContentType.length() == 0) {
		char *ct = NULL;
		CURLcode res = curl_easy_getinfo(mpCURL, CURLINFO_CONTENT_TYPE, &ct);

		if (res == CURLE_OK) {
			if (NULL != ct) {
				mContentType = ct;
			}
			FileLog("httpclient", "HttpClient::HttpHandle, Content-Type:%s", mContentType.c_str());
		}

		res = curl_easy_getinfo(mpCURL, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &mContentLength);
		if (res != CURLE_OK) {
			mContentLength = -1;
		}
		FileLog("httpclient", "HttpClient::HttpHandle, Content-Length:%.0f", mContentLength);
	}

	int len = size * nmemb;
	AddRespondBuffer((const char*)buffer, len);
}

size_t HttpClient::CurlHandle(void *buffer, size_t size, size_t nmemb, void *data) {
	HttpClient *client = (HttpClient *)data;
	client->HttpHandle(buffer, size, nmemb);
	return size * nmemb;
}

size_t HttpClient::CurlProgress(void *data, double downloadTotal, double downloadNow, double uploadTotal, double uploadNow) {
	HttpClient *client = (HttpClient *)data;
	return client->HttpProgress(downloadTotal, downloadNow, uploadTotal, uploadNow);
}

size_t HttpClient::HttpProgress(double downloadTotal, double downloadNow, double uploadTotal, double uploadNow) {
	double totalTime = 0;
	curl_easy_getinfo(mpCURL, CURLINFO_TOTAL_TIME, &totalTime);

	// mark the upload progress
	mdUploadTotal = uploadTotal;
	mdUploadLast = uploadNow;

	// waiting for upload finish, no upload timeout
	if (uploadNow == uploadTotal) {
		if (mdDownloadLast == -1) {
			// update download progress at the beginning
			mdDownloadLast = downloadNow;
			mdDownloadLastTime = totalTime;
		}

		if (mdDownloadLast == downloadNow) {
			if (totalTime - mdDownloadLastTime > DWONLOAD_TIMEOUT) {
				// DWONLOAD_TIMEOUT no receive data, download timeout
				FileLog("httpclient", "HttpClient::HttpProgress, download timeout, timeout:%.2f seconds", DWONLOAD_TIMEOUT);
				mbStop = true;
			}
		} else {
			// update download progress
			mdDownloadLastTime = totalTime;
			mdDownloadLast = downloadNow;
		}
	}

	return mbStop;
}

CURLcode HttpClient::Curl_SSL_Handle(CURL *curl, void *sslctx, void *param) 
{
	/* all set to go */
	return CURLE_OK ;
}
