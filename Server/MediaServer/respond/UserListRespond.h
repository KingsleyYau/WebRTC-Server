/*
 * UserListRespond.h
 *
 *  Created on: 2019/07/23
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef RESPOND_UserListRespond_H_
#define RESPOND_UserListRespond_H_

#include "BaseResultRespond.h"

// ThirdParty
#include <json/json.h>

namespace mediaserver {

class UserListRespond : public BaseResultRespond {
public:
	UserListRespond();
	virtual ~UserListRespond();

	void SetUserList(const list<string> userList);
};
}
#endif /* RESPOND_UserListRespond_H_ */
