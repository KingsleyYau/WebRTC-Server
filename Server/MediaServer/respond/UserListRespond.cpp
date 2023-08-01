/*
 * UserListRespond.cpp
 *
 *  Created on: 2019/07/23
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "UserListRespond.h"

// ThirdParty
#include <json/json.h>

namespace qpidnetwork {

UserListRespond::UserListRespond() {
	// TODO Auto-generated constructor stub
}

UserListRespond::~UserListRespond() {
	// TODO Auto-generated destructor stub
}

void UserListRespond::SetUserList(const list<string>& userList) {
	SetParam(true, "");
	Json::Value userArray = Json::arrayValue;
	for(auto user : userList) {
		userArray.append(user);
	}
	resRoot["count"] = userArray.size();
	resRoot["userlist"] = userArray;
}

} /* namespace qpidnetwork */
