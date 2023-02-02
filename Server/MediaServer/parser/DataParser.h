/*
 * DataParser.h
 *
 *  Created on: 2015-9-28
 *      Author: Max.Chiu
 */

#ifndef PARSER_DATAPARSER_H_
#define PARSER_DATAPARSER_H_

#include "IDataParser.h"
#include <string.h>
namespace mediaserver {
class DataParser : public IDataParser {
public:
	DataParser();
	virtual ~DataParser();

	void SetNextParser(IDataParser *parser);
	virtual int ParseData(char* buffer, int len);
	virtual void Reset();

public:
	void* custom;

private:
	IDataParser *mParser;

};
}
#endif /* PARSER_DATAPARSER_H_ */
