# Copyright (C) 2015 The Mediaserver
# webrtc-tester Makefile
#
# Created on: 2015/10/10
# Author: Max.Chiu
# Email: Kingsleyyau@gmail.com
#

export MAKE	:=	make

ifeq ($(RELEASE), 1)
CXXFLAGS = -O3 
else
CXXFLAGS = -O2 -g
endif

CXXFLAGS += -std=c++11
CXXFLAGS +=	-Wall -fmessage-length=0 -Wunused-variable -Wno-write-strings -Wno-narrowing -Wno-unused-variable 
CXXFLAGS +=	-I. -I../ -I../include -I../third_party \
			-I../third_party/json \
			-I../third_party/libsrtp/include -I../third_party/libsrtp/crypto/include \
			-I../third_party/libsdp/include \
			-I../third_party/libnice/nice -I../third_party/libnice/agent \
			-I/usr/include/glib-2.0/ -I/usr/lib64/glib-2.0/include/ \
			-I/usr/include/mysql \
			-I$(OPENSSL_INCLUDE)

CFLAGS += -DDISABLE_MD5=1

export BUILD_PATH = /root/mediaserver/build
export OPENSSL_INCLUDE = $(BUILD_PATH)/include
export OPENSSL_LIB = $(BUILD_PATH)/lib

# 注意连接依赖顺序问题, 被依赖的放后面
LIBS =		-L. \
			-Wl,-Bstatic -L../crypto -lmediacrypto \
			-Wl,-Bstatic -L../common -lcommon \
			-Wl,-Bstatic -L../socket -lsocket \
			-Wl,-Bstatic -L../media -lmedia \
			-Wl,-Bstatic -L../third_party/json -ljson \
			-Wl,-Bstatic -L../third_party/libsrtp -lsrtp2 \
			-Wl,-Bstatic -L../third_party/libsdp -lsdp \
			-Wl,-Bstatic -L../third_party/libnice/nice/.libs -lnice \
			-Wl,-Bstatic -L../third_party/libnice/stun/.libs -lstun \
			-Wl,-Bstatic -L../third_party/libnice/agent/.libs -lagent \
			-Wl,-Bstatic -L../third_party/libnice/random/.libs -lnice-random \
			-Wl,-Bstatic -L../third_party/libnice/socket/.libs -lsocket \
			-Wl,-Bdynamic -lgio-2.0 -lgobject-2.0 -lgthread-2.0 -lglib-2.0 \
			-Wl,-Bstatic -L$(OPENSSL_LIB) -lssl -lcrypto \
			-Wl,-Bdynamic -ldl -pthread -lidn -lrt -lresolv -lselinux -lpcre
				
OBJS += WebRTC.o IceClient.o
OBJS += DtlsSession.o RtpSession.o RtpRawClient.o MainLoop.o
OBJS +=	WebRTCTester.o webrtc-tester.o mongoose.o
TARGET = webrtc-tester

TARGET_JSON_LIBS = -L. \
			-Wl,-Bstatic -L../common -lcommon \
			-Wl,-Bstatic -L../third_party/json -ljson \
			-Wl,-Bdynamic -ldl -pthread 
			
TARGET_JSON_OBJS = json-tester.o
TARGET_JSON = json-tester

TARGET_HTTPD_LIBS = -L. \
			-Wl,-Bstatic -L../server -lserver \
			-Wl,-Bstatic -L../parser -lparser \
			-Wl,-Bstatic -L../request -lrequest \
            -Wl,-Bstatic -L../respond -lrespond \
			-Wl,-Bstatic -L../common -lcommon \
			-Wl,-Bstatic -L../third_party/libev/.libs -lev \
			-Wl,-Bstatic -L../third_party/json -ljson \
			-Wl,-Bdynamic -ldl -pthread 
			
TARGET_HTTPD_OBJS = httpd.o
TARGET_HTTPD = httpd

TARGET_CAM_LIBS = -L. \
			-Wl,-Bstatic -L../crypto -lmediacrypto \
			-Wl,-Bstatic -L../common -lcommon \
			-Wl,-Bstatic -L../third_party/json -ljson \
			-Wl,-Bstatic -L../third_party/libev/.libs -lev \
			-Wl,-Bstatic -L$(OPENSSL_LIB) -lssl -lcrypto \
			-Wl,-Bdynamic -ldl -pthread 
TARGET_CAM_OBJS = cam-tester.o CamTester.o mongoose.o
TARGET_CAM = cam-tester

.PHONY: clean $(TARGET) ${TARGET_JSON} $(TARGET_HTTPD) $(TARGET_CAM)
	
$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid webrtc-tester completed!'
	@echo ''
	@echo '################################################################'
	
$(TARGET_JSON): $(TARGET_JSON_OBJS)
	$(CXX) -o $(TARGET_JSON) $(TARGET_JSON_OBJS) $(TARGET_JSON_LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid json-tester completed!'
	@echo ''
	@echo '################################################################'
	
$(TARGET_HTTPD): $(TARGET_HTTPD_OBJS)
	$(CXX) -o $(TARGET_HTTPD) $(TARGET_HTTPD_OBJS) $(TARGET_HTTPD_LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid httpd completed!'
	@echo ''
	@echo '################################################################'
		
$(TARGET_CAM): $(TARGET_CAM_OBJS)
	$(CXX) -o $(TARGET_CAM) $(TARGET_CAM_OBJS) $(TARGET_CAM_LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid cam-tester completed!'
	@echo ''
	@echo '################################################################'		
			
all: $(TARGET) $(TARGET_JSON) $(TARGET_HTTPD) $(TARGET_CAM)

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET_JSON) $(TARGET_HTTPD) $(TARGET_CAM)