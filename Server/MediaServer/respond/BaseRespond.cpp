/*
 * BaseRespond.cpp
 *
 *  Created on: 2016年3月11日
 *      Author: max
 */

#include "BaseRespond.h"
#include <common/LogManager.h>
#include <json/json.h>
namespace qpidnetwork {

BaseRespond::BaseRespond() {
	// TODO Auto-generated constructor stub
	statusCode = 200;
}

BaseRespond::~BaseRespond() {
	// TODO Auto-generated destructor stub
}

void BaseRespond::SetStatusCode(unsigned statusCode) {
	this->statusCode = statusCode;
}

unsigned BaseRespond::StatusCode() const {
	return statusCode;
}

string BaseRespond::StatusMsg() const {
	switch (statusCode) {
	case 200:return "OK";break;
	case 400:return "Bad Request";break;
	case 403:return "Forbidden";break;
	case 405:return "Method Not Allowed";break;
	case 406:return "Not Acceptable";break;
	case 408:return "Request Timeout";break;
	case 429:return "Too Many Requests";break;
	default:return "Bad Request";
	}
}

string BaseRespond::Result() const {
	return "";
}
} /* namespace qpidnetwork */
