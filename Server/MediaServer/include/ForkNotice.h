/*
 * ForkNotice.h
 *
 *  Created on: 2020/11/26
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef INCLUDE_FORKNOTICE_H_
#define INCLUDE_FORKNOTICE_H_


namespace mediaserver {
class ForkNotice {
public:
	virtual ~ForkNotice(){};
	virtual void OnForkBefore() = 0;
	virtual void OnForkParent() = 0;
	virtual void OnForkChild() = 0;
};

}


#endif /* INCLUDE_FORKNOTICE_H_ */
