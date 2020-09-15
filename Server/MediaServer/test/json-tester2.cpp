/*
 * dbtest.cpp
 *
 *  Created on: 2015-1-14
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include <string>
using namespace std;

// ThirdParty
#include <json/json.h>
// Common
#include <common/LogManager.h>
#include <common/Arithmetic.h>
#include <common/StringHandle.h>
#include <database/DBSpool.hpp>

bool Parse(int argc, char *argv[]);
void SignalFunc(int sign_no);
string SqlTransferInsert(const string& sql);

class A {
public:
	virtual ~A(){ printf("# ~A \n"); };
};
class B : public A {
public:
	B(){ printf("# B \n"); };
};
int main(int argc, char *argv[]) {
	printf("############## webrtc-test ############## \n");
	Parse(argc, argv);
	srand(time(0));

	/* Ignore */
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGPIPE, &sa, 0);

	/* Handle */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SignalFunc;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);

//	sigaction(SIGHUP, &sa, 0);
	sigaction(SIGINT, &sa, 0); // Ctrl-C
	sigaction(SIGQUIT, &sa, 0);
	sigaction(SIGILL, &sa, 0);
	sigaction(SIGABRT, &sa, 0);
	sigaction(SIGFPE, &sa, 0);
	sigaction(SIGBUS, &sa, 0);
	sigaction(SIGSEGV, &sa, 0);
	sigaction(SIGSYS, &sa, 0);
	sigaction(SIGTERM, &sa, 0);
	sigaction(SIGXCPU, &sa, 0);
	sigaction(SIGXFSZ, &sa, 0);
	// 回收子进程
	sigaction(SIGCHLD, &sa, 0);

	srand(time(0));

	bool bFlag;
	DBSpool dbReadSpool;
	bFlag = dbReadSpool.SetDBparm(
			"192.168.8.177",
			3306,
			"mysqldb_chnlove",
			"dbu_chnlove",
			"2nih@wf$6i"
			);
	bFlag = bFlag && dbReadSpool.Connect();
	printf("# Databse Read Connect, bFlag: %d \n", bFlag);

	DBSpool dbSpool;
	bFlag = dbSpool.SetDBparm(
			"192.168.8.177",
			3306,
			"mysqldb_email",
			"dbu_email",
			"&3u*98043u(ier"
			);
	bFlag = bFlag && dbSpool.Connect();
	printf("# Databse Connect, bFlag: %d \n", bFlag);

	string info;
	char sql[4096] = {0};
	snprintf(sql, sizeof(sql) - 1, "select at_content_en from admire_template where at_code='P729365-A4';");
//	snprintf(sql, sizeof(sql) - 1, "select info from msg_process_list_json where id = 118;");
	printf("# Databse Read, sql: %s \n", sql);
	MYSQL_RES *res;
	int line = 0;
	short ident = 0;
	if ( SQL_TYPE_SELECT == dbReadSpool.ExecuteSQL(sql, &res, ident, line) ) {
		printf("# Databse Read, line: %d \n", line);
		if ( line > 0 ) {
			MYSQL_ROW row;
			mysql_fetch_fields(res);
			if ((row = mysql_fetch_row(res)) != NULL) {
				info = row[0];
				printf("# Databse Read OK, info: %s \n\n", info.c_str());
			}
		}
	}
	dbReadSpool.ReleaseConnection(ident);

	string input;
//	input = "[{\"admireId\":\"IDBEJGGIF\",\"admireInfo\":\"Feed my kitty\\ud83d\\ude03\\ud83d\\udc8b\\ud83d\\udc97 this Halloween!  Have you anything to feed me with \\ud83c\\udf4c\\ud83c\\udf4c\\ud83c\\udf4c? Have you anything to hide in my hungry kitty\\ud83d\\udc31?\",\"birthday\":\"1977-11-15\",\"country\":\"Ukraine\",\"firstname\":\"Anna\",\"height\":\"177\",\"id\":\"83665\",\"lastname\":\"Shulga\",\"marry\":\"1\",\"owner\":\"C765\",\"province\":\"Berdiansk\",\"send_time\":\"2019-10-29 09:22:08\",\"template_type\":\"B\",\"weight\":\"70\",\"womanid\":\"C593741\"}]";
	input = "[{\"admireId\":\"BDBFHGFID\",\"admireInfo\":\"<p align=\\\"left\\\">Hi&nbsp;John,</p><br />I do believe, life isn't just an existence! It's a beautiful miracle of love and joy. We just need to find these.\\r\\nFor the world you may be just the one person, \",\"birthday\":\"1992-01-04\",\"country\":\"Ukraine\",\"firstname\":\"Marina\",\"height\":\"160\",\"id\":\"58767\",\"lastname\":\"Gerasimova\",\"marry\":\"1\",\"owner\":\"C885\",\"province\":\"Luhansk\",\"send_time\":\"2019-11-06 01:37:17\",\"template_type\":\"A\",\"weight\":\"49\",\"womanid\":\"C597482\"},{\"admireId\":\"FDBFHHBBG\",\"admireInfo\":\"U know,I don't understand why you're not married because you're a man who I think can take the heart of every girl!my heart is excited!U know why!\",\"birthday\":\"1989-06-05\",\"country\":\"Ukraine\",\"firstname\":\"Mary\",\"height\":\"170\",\"id\":\"82461\",\"lastname\":\"Kupina\",\"marry\":\"1\",\"owner\":\"C1280\",\"province\":\"Kiev\240(Kyiv)\",\"send_time\":\"2019-11-06 01:29:54\",\"template_type\":\"B\",\"weight\":\"53\",\"womanid\":\"C635377\"},{\"admireId\":\"IDBFHHAHJ\",\"admireInfo\":\"You can be my second half and I even can not imagine that you will not answer me. You're already typing the letter for me, I'm right? :-)\",\"birthday\":\"1979-02-27\",\"country\":\"Ukraine\",\"firstname\":\"Marina \",\"height\":\"167\",\"id\":\"86628\",\"lastname\":\"Shorop\",\"marry\":\"3\",\"owner\":\"C1417\",\"province\":\"Odessa\",\"send_time\":\"2019-11-06 00:54:06\",\"template_type\":\"B\",\"weight\":\"50\",\"womanid\":\"C309051\"},{\"admireId\":\"DDBFHEIBJ\",\"admireInfo\":\"<p align=\\\"left\\\">Hi&nbsp;John,</p><br />You probably every day get a lot of letters here and probably very difficult to understand what kind of woman could be your second half.\\r\\nI understand you, becau\",\"birthday\":\"1985-04-13\",\"country\":\"Ukraine\",\"firstname\":\"Olga\",\"height\":\"173\",\"id\":\"61392\",\"lastname\":\"Kondratevich\",\"marry\":\"1\",\"owner\":\"C1119\",\"province\":\"Kiev\240(Kyiv)\",\"send_time\":\"2019-11-06 00:06:29\",\"template_type\":\"A\",\"weight\":\"52\",\"womanid\":\"C102940\"},{\"admireId\":\"DDBFHIEHH\",\"admireInfo\":\"How are you today? I'm fine )) just not enough man with whom I can share the warmth of my heart*Two-Hearts*How are you today? I'm fine )) just not enough man with whom I can share the warmth of my hea\",\"birthday\":\"1997-08-22\",\"country\":\"Ukraine\",\"firstname\":\"Emma\",\"height\":\"168\",\"id\":\"96076\",\"lastname\":\"Dobryden\",\"marry\":\"1\",\"owner\":\"C2527\",\"province\":\"Luhansk\",\"send_time\":\"2019-11-06 04:13:26\",\"template_type\":\"B\",\"weight\":\"57\",\"womanid\":\"C538721\"}]";
	//	string input = "[{\"admireId\":\"IDBEJGGIF\",\"admireInfo\":\"Feed my kitty😃💋💗 this Halloween!  Have you anything to feed me with 🍌🍌🍌? Have you anything to hide in my hungry kitty🐱?\",\"birthday\":\"1977-11-15\",\"country\":\"Ukraine\",\"firstname\":\"Anna\",\"height\":\"177\",\"id\":\"83665\",\"lastname\":\"Shulga\",\"marry\":\"1\",\"owner\":\"C765\",\"province\":\"Berdiansk\",\"send_time\":\"2019-10-29 09:22:08\",\"template_type\":\"B\",\"weight\":\"70\",\"womanid\":\"C593741\"}]";
//	string input = "{\"key\":\"😄😅😆😉\"}";
//	input = "\"😄😅😆😉\"";
//	input = "\"\\ud83d\\ude04\\ud83d\\ude05\\ud83d\\ude06\\ud83d\\ude09\"";
//	input = "\"汉字\"";
//	input = "{\"admireId\":\"HDIHJIJ\",\"admireInfo\":\"\\u00f0\\u0178\\u02dc\\u201e \\u00f0\\u0178\\u02dc\\u2026 \\u00f0\\u0178\\u02dc\\u2020 \\u00f0\\u0178\\u02dc\\u2030,ues'\\\",\\/,kwg kwg kwg \\r\\nsdfljsd@#$%^ alkjf sdj flksd \\r\\nlsdkjf lsdjflsdflsdj fsdf ssd sdfsdfsdkjsd fljsdl fjsd fsd\",\"birthday\":\"1989-10-06\",\"country\":\"China\",\"firstname\":\"nicole\",\"height\":\"160\",\"id\":\"43748\",\"lastname\":\"nicole\",\"marry\":\"1\",\"owner\":\"GZA\",\"province\":\"Guangdong\",\"send_time\":\"2019-11-01 08:12:43\",\"template_type\":\"B\",\"weight\":\"40\",\"womanid\":\"P503382\"}";
	string output = info;
	Json::Value reqRoot;
	Json::Reader reader;
	bool bParse = reader.parse(input, reqRoot, false);
	if ( bParse ) {
		printf("# Json Parse OK, input: %s \n\n", input.c_str());
//		Arithmetic ari;
//		string inputHex = ari.AsciiToHexWithSep((char *)info.c_str(), info.length());
//		printf("# Json Read OK, inputHex: %s \n\n", inputHex.c_str());

		int i = 0;
		Json::Value infoNew;
		infoNew["birthday"] 		= "";
		infoNew["firstname"] 		= "";
		infoNew["lastname"]  		= "";
		infoNew["country"]   		= "";
		infoNew["owner"]     		= "";
		infoNew["id"]        		= "";
		infoNew["womanid"]   		= "C239420";
		infoNew["height"]   		= "";
		infoNew["weight"]   		= "";
		infoNew["marry"]     		= "";
		infoNew["admireInfo"]		= "People have very different character and we can like it or not. Everyone has something special in it, so we need to deal with it.";
		infoNew["admireId"]			= "ABCD1234";
		infoNew["province"]  		= "";
		infoNew["template_type"]  	= "B";
		infoNew["send_time"]  		= "";

		reqRoot.append(infoNew);
//		reqRoot[i]["admireInfo"] = info;
//		reqInfo.append(reqRoot[i]);
//		Json::Value reqInfo = reqInfo0['admireInfo'];
//		string reqInfo = reqInfo0['admireInfo'].asString();
		Json::FastWriter writer(true);
		output = writer.write(reqRoot);
		printf("# Json Write OK, output: %s \n", output.c_str());

//		string outputHex = ari.AsciiToHexWithSep((char *)output.c_str(), output.length());
//		printf("# Write OK, outputHex: %s \n\n", outputHex.c_str());
	}

//	snprintf(sql, sizeof(sql) - 1, "update msg_process_list_json set info = '%s' where id = 255;\n", SqlTransferInsert(input).c_str());
////	snprintf(sql, sizeof(sql) - 1, "update msg_process_list_json set info = '%s' where id = 255;\n", SqlTransferInsert(output).c_str());
//	printf("# Databse Write, sql: %s \n", sql);
//	if ( SQL_TYPE_UPDATE == dbSpool.ExecuteSQL(sql, &res, ident, line) ) {
//		printf("# Databse Write OK, line: %d \n", line);
//	}
//	dbSpool.ReleaseConnection(ident);

	return EXIT_SUCCESS;
}

bool Parse(int argc, char *argv[]) {
	string key;
	string value;

	for( int i = 1; (i + 1) < argc; i+=2 ) {
		key = argv[i];
		value = argv[i+1];
	}

	return true;
}

void SignalFunc(int sign_no) {
	switch(sign_no) {
	default:{
		LogAync(
				LOG_ALERT, "main( Get signal : %d )", sign_no
				);
		LogManager::GetLogManager()->LogFlushMem2File();
		signal(sign_no, SIG_DFL);
		kill(getpid(), sign_no);
	}break;
	}
}

string SqlTransferInsert(const string& sql) {
	string result = "";

	result = StringHandle::replace(sql, "\\", "\\\\");
	result = StringHandle::replace(result, "'", "\\'");

	return result;
}
