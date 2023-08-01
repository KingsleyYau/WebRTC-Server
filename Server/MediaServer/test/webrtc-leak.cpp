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

#include <mcheck.h>
#include <malloc.h>

#include <string>
using namespace std;

#include <webrtc/WebRTC.h>

// glib
#include <glib-object.h>
#include <gio/gio.h>
#include <gio/gnetworking.h>

using namespace qpidnetwork;

bool Parse(int argc, char *argv[]);
void SignalFunc(int sign_no);

int main(int argc, char *argv[]) {
	printf("############## webrtc-leak ############## \n");
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

//	// 禁止malloc调用mmap分配内存
//	mallopt(M_MMAP_MAX, 0);
//	// 禁止内存缩进，sbrk申请的内存释放后不会归还给操作系统
//	mallopt(M_TRIM_THRESHOLD, 0);

	char c;

	LogManager::GetLogManager()->Start(LOG_INFO, "log");
	LogManager::GetLogManager()->SetDebugMode(false);
	LogManager::GetLogManager()->LogSetFlushBuffer(1 * BUFFER_SIZE_1K * BUFFER_SIZE_1K);
	WebRTC::GobalInit("./ssl/tester.crt", "./ssl/tester.key", "192.168.88.133", "", true, "", "", "mediaserver12345");

	setenv("MALLOC_TRACE","trace.log", 1);

//	printf("please input any key to continue.\n");
//	scanf("%c", &c);
	string sdp = "\
v=0\n\
o=- 8792925737725123967 2 IN IP4 127.0.0.1\n\
s=-\n\
t=0 0\n\
a=group:BUNDLE 0 1\n\
a=msid-semantic: WMS\n\
m=audio 34298 UDP/TLS/RTP/SAVPF 111\n\
c=IN IP4 172.25.32.133\n\
a=rtcp:9 IN IP4 0.0.0.0\n\
a=candidate:4 1 UDP 335544831 172.25.32.133 34298 typ relay raddr 192.168.88.134 rport 9\n\
a=ice-ufrag:ZXwm\n\
a=ice-pwd:33uzXj7T5M+FYeDVS7D5eh\n\
a=ice-options:trickle\n\
a=fingerprint:sha-256 8A:9B:88:96:34:6C:5F:6D:E5:ED:A3:B5:58:60:15:C6:83:1A:00:79:E6:61:8A:8D:70:69:C5:6B:4E:71:82:35\n\
a=setup:actpass\n\
a=mid:0\n\
a=sendonly\n\
a=rtcp-mux\n\
a=rtpmap:111 opus/48000/2\n\
a=fmtp:111 minptime=10;useinbandfec=1\n\
a=ssrc:305419897 cname:audio\n\
m=video 9 UDP/TLS/RTP/SAVPF 102\n\
c=IN IP4 0.0.0.0\n\
a=rtcp:9 IN IP4 0.0.0.0\n\
a=ice-ufrag:ZXwm\n\
a=ice-pwd:33uzXj7T5M+FYeDVS7D5eh\n\
a=ice-options:trickle\n\
a=fingerprint:sha-256 8A:9B:88:96:34:6C:5F:6D:E5:ED:A3:B5:58:60:15:C6:83:1A:00:79:E6:61:8A:8D:70:69:C5:6B:4E:71:82:35\n\
a=setup:actpass\n\
a=mid:1\n\
a=sendonly\n\
a=rtcp-mux\n\
a=rtcp-rsize\n\
a=rtpmap:102 H264/90000\n\
a=fmtp:102 packetization-mode=1;profile-level-id=42e01f\n\
a=ssrc:305419896 cname:video\n\
";

	int totalSize = 40;
	int totalTimes = 1000;
	WebRTC *rtcs = NULL;
	IceClient *ices = NULL;
	DtlsSession *dtlss = NULL;

	mtrace();
	rtcs = new WebRTC[totalSize];
//	ices = new IceClient[totalSize];
//	dtlss = new DtlsSession[totalSize];
//	RtpRawClient raws[100];
	for(int i = 0, port = 10000; i < totalSize; i++, port +=4) {
		rtcs[i].Init(
				"",
				"",
				"127.0.0.1", port,
				"127.0.0.1", port + 2,
				"127.0.0.1", port
				);
//		raws[i].Init("", -1, "127.0.0.1", port);
	}
	while (true) {
		for (int t = 0; t < totalTimes; t++) {
			for(int i = 0; i < totalSize; i++) {
				string url = "rtmp://192.168.88.133:4000/cdn_standard/" + to_string(i);
//				bool bFlag = rtcs[i].ParseRemoteSdp(sdp);
				rtcs[i].Start(sdp, url);
//				ices[i].Start("mediaserver");
//				dtlss[i].Start();
//				raws[i].Start(NULL, 0, NULL, 0);
			}

			usleep(100*1000);

			for(int i = 0; i < totalSize; i++) {
//				rtcs[i].Stop();
//				ices[i].Stop();
//				dtlss[i].Stop();
//				raws[i].Stop();
			}
			printf("times: %d\n", t);
		}

		usleep(10000*1000);

		for(int i = 0; i < totalSize; i++) {
			rtcs[i].Stop();
//			ices[i].Stop();
		}

		//		printf("please input any key to continue.\n");
		//		scanf("%c", &c);
		break;
	}

	if ( rtcs ) {
		delete[] rtcs;
	}
	if ( ices ) {
		delete[] ices;
	}
	if ( dtlss ) {
		delete[] dtlss;
	}
	muntrace();

	LogManager::GetLogManager()->LogFlushMem2File();

	printf("please input any key to exit.\n");
	scanf("%c", &c);

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
		printf("# main( sign_no: %d ) \n", sign_no);
//		LogManager::GetLogManager()->LogFlushMem2File();
		exit(0);
	}break;
	}
}
