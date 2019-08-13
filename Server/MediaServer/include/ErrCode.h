/*
 * ErrcodeHeader.h
 *
 *  Created on: 2019/08/13
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef INCLUDE_ERRCODE_H_
#define INCLUDE_ERRCODE_H_

typedef enum RequestErrorType {
	RequestErrorType_None = 0,
	RequestErrorType_Unknow_Error,
	RequestErrorType_Request_Data_Format_Parse,
	RequestErrorType_Request_Unknow_Command,
	RequestErrorType_Request_Missing_Param,
	RequestErrorType_WebRTC_Start_Fail,
	RequestErrorType_WebRTC_No_More_WebRTC_Connection_Allow,
	RequestErrorType_WebRTC_Update_Candidate_Before_Call,
} RequestErrorType;

const string RequestErrorTypeMsg[] = {
	"",
	"Unknow Error."
	"Request Data Format Parse Error."
	"Request Unknow Command Error.",
	"Request Missing Param Error.",
	"WebRTC Start Fail Error.",
	"WebRTC No More Connection Allow Error.",
	"WebRTC Update Candidate Before Call Error.",
};

#endif /* INCLUDE_ERRCODE_H_ */
