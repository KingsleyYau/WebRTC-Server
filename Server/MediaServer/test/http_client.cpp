/*
 * dbtest.cpp
 *
 *  Created on: 2015-1-14
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include <HttpServerTest.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <string>
using namespace std;

char ip[128] = {"127.0.0.1"};
int iPort = 9201;
int iCurrent = 0;
int iTotal = 100000;

bool Parse(int argc, char *argv[]);

int main(int argc, char *argv[]) {
	printf("############## client ############## \n");
	Parse(argc, argv);
	srand(time(0));

	HttpServerTest test;
	test.Start(ip, iPort, iTotal, 2);

	while( true ) {
		sleep(3);
	}

	return EXIT_SUCCESS;
}

bool Parse(int argc, char *argv[]) {
	string key;
	string value;

	for( int i = 1; (i + 1) < argc; i+=2 ) {
		key = argv[i];
		value = argv[i+1];

		if( key.compare("-h") == 0 ) {
			memcpy(ip, value.c_str(), value.length());
		} else if( key.compare("-p") == 0 ) {
			iPort = atoi(value.c_str());
		} else if( key.compare("-n") == 0 ) {
			iTotal = atoi(value.c_str());
		}
	}

//	printf("# ip : %s, iPort : %d\n", ip, iPort);

	return true;
}
