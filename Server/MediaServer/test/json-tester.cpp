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

	DBSpool dbSpool;
	bool bFlag = dbSpool.SetDBparm(
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
	snprintf(sql, sizeof(sql) - 1, "select info from msg_process_list_json where id = 256;");
	printf("# Databse Read, sql: %s \n", sql);

	MYSQL_RES *res;
	int line = 0;
	short ident = 0;

	if ( SQL_TYPE_SELECT == dbSpool.ExecuteSQL(sql, &res, ident, line) ) {
		printf("# Databse Read, line: %d \n", line);
		if ( line > 0 ) {
			MYSQL_ROW row;
			mysql_fetch_fields(res);
			if ((row = mysql_fetch_row(res)) != NULL) {
				info = row[0];
				printf("# Databse Read, info: %s \n\n", info.c_str());
			}
		}
	}
	dbSpool.ReleaseConnection(ident);

//	string input = "[{\"admireId\":\"IDBEJGGIF\",\"admireInfo\":\"Feed my kitty\\ud83d\\ude03\\ud83d\\udc8b\\ud83d\\udc97 this Halloween!  Have you anything to feed me with \\ud83c\\udf4c\\ud83c\\udf4c\\ud83c\\udf4c? Have you anything to hide in my hungry kitty\\ud83d\\udc31?\",\"birthday\":\"1977-11-15\",\"country\":\"Ukraine\",\"firstname\":\"Anna\",\"height\":\"177\",\"id\":\"83665\",\"lastname\":\"Shulga\",\"marry\":\"1\",\"owner\":\"C765\",\"province\":\"Berdiansk\",\"send_time\":\"2019-10-29 09:22:08\",\"template_type\":\"B\",\"weight\":\"70\",\"womanid\":\"C593741\"}]";
	string input = "[{\"admireId\":\"IDBEJGGIF\",\"admireInfo\":\"Feed my kitty😃💋💗 this Halloween!  Have you anything to feed me with 🍌🍌🍌? Have you anything to hide in my hungry kitty🐱?\",\"birthday\":\"1977-11-15\",\"country\":\"Ukraine\",\"firstname\":\"Anna\",\"height\":\"177\",\"id\":\"83665\",\"lastname\":\"Shulga\",\"marry\":\"1\",\"owner\":\"C765\",\"province\":\"Berdiansk\",\"send_time\":\"2019-10-29 09:22:08\",\"template_type\":\"B\",\"weight\":\"70\",\"womanid\":\"C593741\"}]";
//	string input = info;
	string output;
	Json::Value reqRoot;
	Json::Reader reader;
	bool bParse = reader.parse(input, reqRoot, false);
	if ( bParse ) {
		Arithmetic ari;
		string inputHex = ari.AsciiToHexWithSep((char *)input.c_str(), input.length());
		printf("# Read OK, input: %s \n\n", input.c_str());
//		printf("# Read OK, inputHex: %s \n\n", inputHex.c_str());

		Json::FastWriter writer(false);

		int i = 0;
		Json::Value reqInfo;
		reqInfo.append(reqRoot[i]);
//		Json::Value reqInfo = reqInfo0['admireInfo'];
//		string reqInfo = reqInfo0['admireInfo'].asString();
		output = writer.write(reqInfo);
		string outputHex = ari.AsciiToHexWithSep((char *)output.c_str(), output.length());

		printf("# Write OK, output: %s \n\n", output.c_str());
		printf("# Write OK, outputHex: %s \n\n", outputHex.c_str());
	}

	snprintf(sql, sizeof(sql) - 1, "update msg_process_list_json set info = '%s' where id = 255;", SqlTransferInsert(output).c_str());
	printf("# Databse Write, sql: %s \n", sql);
	if ( SQL_TYPE_UPDATE == dbSpool.ExecuteSQL(sql, &res, ident, line) ) {
		printf("# Databse Write, line: %d \n", line);
		if ( line > 0 ) {
			printf("# Databse Write, OK \n\n");
		}
	}
	dbSpool.ReleaseConnection(ident);

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
				LOG_ERR_SYS, "main( Get signal : %d )", sign_no
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
