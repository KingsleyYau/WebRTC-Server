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

typedef struct ErrObject {
	ErrObject(
			RequestErrorType errType,
			int errNo,
			string errMsg
			) {
		this->errType = errType;
		this->errNo = errNo;
		this->errMsg = errMsg;
	}

	RequestErrorType errType;
	int errNo;
	string errMsg;
} ErrObject;

const ErrObject RequestErrObjects[] = {
		ErrObject(RequestErrorType_None, RequestErrorType_None, ""),
		ErrObject(RequestErrorType_Unknow_Error, RequestErrorType_Unknow_Error, "Unknow Error."),
		ErrObject(RequestErrorType_Request_Data_Format_Parse, RequestErrorType_Unknow_Error, "Request Data Format Parse Error."),
		ErrObject(RequestErrorType_Request_Unknow_Command, RequestErrorType_None, "Request Missing Param Error."),
		ErrObject(RequestErrorType_Request_Missing_Param, RequestErrorType_Unknow_Error, "Request Missing Param Error."),
		ErrObject(RequestErrorType_WebRTC_Start_Fail, RequestErrorType_Unknow_Error, "WebRTC Start Fail Error."),
		ErrObject(RequestErrorType_WebRTC_No_More_WebRTC_Connection_Allow, RequestErrorType_None, "WebRTC No More Connection Allow Error."),
		ErrObject(RequestErrorType_WebRTC_Update_Candidate_Before_Call, RequestErrorType_Unknow_Error, "WebRTC Update Candidate Before Call Error."),
};

#endif /* INCLUDE_ERRCODE_H_ */
