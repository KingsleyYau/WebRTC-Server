/*
 * DataParser.cpp
 *
 *  Created on: 2015-9-28
 *      Author: Max
 */

#include "DataParser.h"
namespace mediaserver {
DataParser::DataParser() {
	// TODO Auto-generated constructor stub
	mParser = NULL;
	custom = NULL;
}

DataParser::~DataParser() {
	// TODO Auto-generated destructor stub
}

void DataParser::SetNextParser(IDataParser *parser) {
	mParser = parser;
}

int DataParser::ParseData(char* buffer, int len) {
	if (mParser != NULL) {
		return mParser->ParseData(buffer, len);
	}
	return len;
}

void DataParser::Reset() {
	if (mParser != NULL) {
		return mParser->Reset();
	}
}
}
