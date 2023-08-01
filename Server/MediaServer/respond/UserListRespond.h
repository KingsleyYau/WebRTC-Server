/*
 * UserListRespond.h
 *
 *  Created on: 2019/07/23
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RESPOND_USERLISTRESPOND_H_
#define RESPOND_USERLISTRESPOND_H_

#include "BaseResultRespond.h"
// ThirdParty
#include <json/json.h>

namespace qpidnetwork {

class UserListRespond : public BaseResultRespond {
public:
	UserListRespond();
	virtual ~UserListRespond();

	void SetUserList(const list<string>& userList);
};
}
#endif /* RESPOND_USERLISTRESPOND_H_ */
