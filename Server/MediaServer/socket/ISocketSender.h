/*
 * ISocketSender.h
 *
 *  Created on: 2019/07/02
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef SOCKET_ISOCKETSENDER_H_
#define SOCKET_ISOCKETSENDER_H_

namespace qpidnetwork {
    class SocketSender {
    public:
        virtual ~SocketSender(){};
        virtual int SendData(const void *data, unsigned int len) = 0;
    };
} /* namespace qpidnetwork */

#endif /* SOCKET_ISOCKETSENDER_H_ */
