/*
 * Client.h
 *
 *  Created on: 2016-9-23
 *      Author: max
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef SERVER_CLIENT_H_
#define SERVER_CLIENT_H_

#include "Socket.h"

#include <parser/IDataParser.h>

#include <common/Buffer.h>
#include <common/KMutex.h>

#include <common/LogManager.h>

using namespace std;

namespace mediaserver {
/**
 * 每次读取数据Buffer Size
 */
#define READ_BUFFER_SIZE 1024
/**
 * 总读包缓存Buffer Size
 */
#define CLIENT_BUFFER_SIZE 1 * 64 * 1024

class Client {
public:
	static Client* Create() {
		Client* client = new Client();
		return client;
	}

	static void Destroy(Client* client) {
		if( client ) {
			delete client;
		}
	}

public:
	Client() :
		clientMutex(KMutex::MutexType_Recursive)
	{
		this->buffer = new Buffer(CLIENT_BUFFER_SIZE);
		Reset();
	}

	~Client() {
	}

	void Reset() {
		socket = NULL;
		buffer->Reset();

		recvHandleCount = 0;
		disconnected = false;

		parser = NULL;
		data = NULL;
	}

	bool CheckBufferEnough() {
		bool bFlag = (buffer->Freespace() >= READ_BUFFER_SIZE);
		return bFlag;
	}

	bool Write(char *data, int len) {
		int size = buffer->Write(data, len);
		return size > 0;
	}

	void Parse() {
		void *data = NULL;
		int size = 0;
		buffer->ReadZeroCopy((const void **)&data, size);

		if( size > 0 && data ) {
			if( parser ) {
				int parseLen = parser->ParseData((char *)data, size);
				if( parseLen > 0 ) {
					// 去除已经解析的Buffer
					size = buffer->Toss(parseLen);
				}
			}
		}
	}

public:
	/**
	 * 处理队列中数量
	 */
	int recvHandleCount;

	/**
	 * 是否已经断开连接
	 */
	bool disconnected;

	/**
	 * 自定义数据
	 */
	void* data;

	/**
	 * 同步锁
	 */
	KMutex clientMutex;

	/**
	 * socket
	 */
	void *socket;

	/**
	 * 解析器
	 */
	IDataParser* parser;

private:
	/**
	 *	接收到的数据
	 */
	Buffer* buffer;

};

typedef KSafeList<Client*> ClientList;
}
#endif /* SERVER_CLIENT_H_ */
