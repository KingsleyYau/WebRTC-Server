/*
 * HttpEntiy.cpp
 *
 *  Created on: 2014-12-24
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include "HttpEntiy.h"

#include <curl/include/curl/curl.h>
#include <common/KLog.h>
#include <common/CheckMemoryLeak.h>

HttpEntiy::HttpEntiy() {
	// TODO Auto-generated constructor stub
	mAuthorization = "";
	mIsGetMethod = false;
	mbSaveCookie = false;
}

HttpEntiy::~HttpEntiy() {
	// TODO Auto-generated destructor stub

}
void HttpEntiy::AddHeader(string key, string value) {
	HttpMap::iterator itr = mHeaderMap.find(key);
	if (itr != mHeaderMap.end()) {
		itr->second = value;
	} else {
		mHeaderMap.insert(HttpMap::value_type(key, value));
	}
}
void HttpEntiy::SetKeepAlive(bool isKeepAlive) {
	if (isKeepAlive) {
		AddHeader("Connection", "keep-alive");
	} else {
		AddHeader("Connection", "close");
	}
}

void HttpEntiy::SetAuthorization(string user, string password) {
	if (user.length() > 0 && password.length() > 0) {
		mAuthorization = user + ":" + password;
	}
}

void HttpEntiy::SetGetMethod(bool isGetMethod) {
	mIsGetMethod = isGetMethod;
}

void HttpEntiy::SetSaveCookie(bool isSaveCookie) {
	mbSaveCookie = isSaveCookie;
}

void HttpEntiy::SetRawData(string data) {
    mRawData = data;
}

void HttpEntiy::AddContent(string key, string value) {
	HttpMap::iterator itr = mContentMap.find(key);
	if (itr != mContentMap.end()) {
		itr->second = value;
	} else {
		mContentMap.insert(HttpMap::value_type(key, value));
	}
}

void HttpEntiy::AddFile(string key, string fileName, string mimeType) {
	FileMap::iterator itr = mFileMap.find(key);
	if (itr != mFileMap.end()) {
		itr->second.fileName = fileName;
		itr->second.mimeType = mimeType;
	} else {
		FileMapItem item;
		item.fileName = fileName;
		item.mimeType = mimeType;
		mFileMap.insert(FileMap::value_type(key, item));
	}
}

void HttpEntiy::Reset() {
	mFileMap.empty();
	mContentMap.empty();
	mAuthorization = "";
	mIsGetMethod = false;
	mbSaveCookie = false;
}

string HttpEntiy::GetContentDesc() {
    string desc = "";
    string item = "";
    string key = "";
    string value = "";
    for(HttpMap::iterator itr = mContentMap.begin(); itr != mContentMap.end(); itr++) {
        item = itr->first + "=" + itr->second;
        desc += item;
        desc += "&";
    }

    if(desc.length() > 0) {
        desc = desc.substr(0, desc.length() - 1);
    }

    if(mRawData.length() > 0) {
        if(desc.length() > 0) {
            desc += " ";
        }
        desc += "rawData:";
        desc += mRawData;
    }

    return desc;
}
