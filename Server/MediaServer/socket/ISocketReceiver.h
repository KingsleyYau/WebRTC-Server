/*
 * ISocketReceiver.h
 *
 *  Created on: 2019/08/26
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef SOCKET_ISOCKETRECEIVER_H_
#define SOCKET_ISOCKETRECEIVER_H_

namespace mediaserver {
    class SocketReceiver {
    public:
        virtual ~SocketReceiver(){};
        virtual int RecvData(void *buffer, unsigned int size) = 0;
    };
} /* namespace mediaserver */


#endif /* SOCKET_ISOCKETRECEIVER_H_ */
