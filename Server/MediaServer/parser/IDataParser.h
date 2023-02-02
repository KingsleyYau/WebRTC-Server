/*
 * IDataParser.h
 *
 *  Created on: 2015-9-28
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#ifndef PARSER_IDATAPARSER_H_
#define PARSER_IDATAPARSER_H_
#pragma pack(1)
namespace mediaserver {
class IDataParser {
public:
	/*
	 *	return : -1:解析错误/解析使用长度
	 */
	virtual int ParseData(char* buffer, int len) = 0;
	virtual void Reset() = 0;
	virtual ~IDataParser(){};
};
}
#pragma pack()
#endif /* PARSER_IDATAPARSER_H_ */
