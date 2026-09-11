/*
 * JsonParser.h
 *
 *  Created on: 2019/07/24
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#ifndef PARSER_JSONPARSER_H_
#define PARSER_JSONPARSER_H_

#include "DataParser.h"
// ThirdParty
#include <json/json.h>

namespace qpidnetwork {
class JsonParser : public DataParser {
public:
	JsonParser();
	virtual ~JsonParser();

	int ParseData(char* buffer, int len);
};

} /* namespace qpidnetwork */

#endif /* PARSER_JSONPARSER_H_ */
