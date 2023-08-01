# Copyright (C) 2015 The QpidNetwork
# Tester Makefile
#
# Created on: 2015/10/10
# Author: Max.Chiu
# Email: Kingsleyyau@gmail.com
#

export MAKE	:=	make

release=0
ifeq ($(release), 1)
CXXFLAGS = -O3 
else
CXXFLAGS = -g
endif

CXXFLAGS += -std=c++11 -DWEBRTC_POSIX -DWEBRTC_LINUX
CXXFLAGS +=	-Wall -fmessage-length=0 -Wunused-variable -Wno-write-strings -Wno-narrowing -Wno-unused-variable
CXXFLAGS +=	-I. -I../ -I../include -I../third_party \
			-I../third_party/json \
			-I../third_party/curl/include \
			-I../third_party/libsrtp/include -I../third_party/libsrtp/crypto/include \
			-I../third_party/libsdp/include \
			-I../third_party/libnice/nice -I../third_party/libnice/agent \
			-I../third_party/abseil-cpp \
			-I/usr/include/glib-2.0/ -I/usr/lib64/glib-2.0/include/ \
			-I/usr/include/mysql \
			-I$(BUILD_PATH_INCLUDE)

CXXFLAGS += -DDISABLE_MD5=1  -DMG_ENABLE_SSL=1

export BUILD_PATH = /root/mediaserver/build
export BUILD_PATH_INCLUDE = $(BUILD_PATH)/include
export BUILD_PATH_LIB = $(BUILD_PATH)/lib

# 注意连接依赖顺序问题, 被依赖的放后面
LIBS =		-L. \
			-Wl,-Bstatic -L../crypto -lmediacrypto \
			-Wl,-Bstatic -L../webrtc -lwebrtc \
			-Wl,-Bstatic -L../rtp -lrtp \
			-Wl,-Bstatic -L../server -lserver \
			-Wl,-Bstatic -L../crypto -lmediacrypto \
			-Wl,-Bstatic -L../common -lcommon \
			-Wl,-Bstatic -L../socket -lsocket \
			-Wl,-Bstatic -L../media -lmedia \
			-Wl,-Bstatic -L../third_party/mongoose -lmongoose \
			-Wl,-Bstatic -L../third_party/json -ljson \
			-Wl,-Bstatic -L../third_party/libsrtp -lsrtp2 \
			-Wl,-Bstatic -L../third_party/libsdp -lsdp \
			-Wl,-Bstatic -L../third_party/libnice/nice/.libs -lnice \
			-Wl,-Bstatic -L../third_party/libnice/stun/.libs -lstun \
			-Wl,-Bstatic -L../third_party/libnice/agent/.libs -lagent \
			-Wl,-Bstatic -L../third_party/libnice/random/.libs -lnice-random \
			-Wl,-Bstatic -L../third_party/libnice/socket/.libs -lsocket \
			-Wl,-Bstatic -L../third_party/abseil-cpp/absl/types -labsl_bad_optional_access \
			-Wl,-Bstatic -L../third_party/abseil-cpp/absl/types -labsl_bad_variant_access \
			-Wl,-Bstatic -L../third_party/abseil-cpp/absl/types -labsl_bad_any_cast_impl \
			-Wl,-Bdynamic -lgio-2.0 -lgobject-2.0 -lgthread-2.0 -lglib-2.0 \
			-Wl,-Bstatic -L$(BUILD_PATH_LIB) -lssl -lcrypto \
			-Wl,-Bdynamic -ldl -pthread -lidn -lrt -lresolv -lselinux -lpcre
				
OBJS += webrtc-tester.o	WebRTCTester.o WebRTCClient.o
TARGET = webrtc-tester
TARGET_OBJ = webrtc-leak.o
TARGET_LEAK = webrtc-leak

TARGET_CAM_VIEWER_OBJS = cam-viewer.o CamViewer.o
TARGET_CAM_VIEWER = cam-viewer

TARGET_CAM_PUSHER_OBJS = cam-pusher.o CamPusher.o WebRTCClient.o
TARGET_CAM_PUSHER = cam-pusher

TARGET_CAMCHAT_PUSHER_OBJS = camchat-pusher.o CamChatPusher.o WebRTCClient.o
TARGET_CAMCHAT_PUSHER = camchat-pusher

TARGET_GLIB_LIBS = -L. \
			-Wl,-Bstatic -L../common -lcommon \
			-Wl,-Bstatic -L../third_party/json -ljson \
			-Wl,-Bdynamic -lgio-2.0 -lgobject-2.0 -lgthread-2.0 -lglib-2.0 \
			-Wl,-Bdynamic -ldl -pthread 
			
TARGET_GLIB_OBJS = glib-tester.o
TARGET_GLIB = glib-tester

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

TARGET_HTTPC_LIBS = -L. \
			-Wl,-Bstatic -L../httpclient -lhttpclient \
			-Wl,-Bstatic -L../common -lcommon \
			-Wl,-Bstatic -L../third_party/curl/lib/.libs -lcurl \
			-Wl,-Bstatic -L../third_party/json -ljson \
			-Wl,-Bstatic -L$(BUILD_PATH_LIB) -lssl -lcrypto \
			-Wl,-Bdynamic -ldl -pthread -lidn -lrt -lresolv -lselinux -lpcre -luuid -lz
			
TARGET_HTTPC_OBJS = httpc.o
TARGET_HTTPC = httpc

.PHONY: clean all $(TARGET) $(TARGET_LEAK) ${TARGET_GLIB} $(TARGET_HTTPD) $(TARGET_HTTPC) $(TARGET_CAM_VIEWER) $(TARGET_CAM_PUSHER) $(TARGET_WSCAT)
	
$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid webrtc-tester completed!'
	@echo ''
	@echo '################################################################'
	
$(TARGET_LEAK): $(TARGET_OBJ)
	$(CXX) -o $(TARGET_LEAK) $(TARGET_OBJ) $(LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid webrtc-leak completed!'
	@echo ''
	@echo '################################################################'
		
$(TARGET_GLIB): $(TARGET_GLIB_OBJS)
	$(CXX) -o $(TARGET_GLIB) $(TARGET_GLIB_OBJS) $(TARGET_GLIB_LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid glib-tester completed!'
	@echo ''
	@echo '################################################################'
		
$(TARGET_HTTPD): $(TARGET_HTTPD_OBJS)
	$(CXX) -o $(TARGET_HTTPD) $(TARGET_HTTPD_OBJS) $(TARGET_HTTPD_LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid httpd completed!'
	@echo ''
	@echo '################################################################'
	
$(TARGET_HTTPC): $(TARGET_HTTPC_OBJS)
	$(CXX) -o $(TARGET_HTTPC) $(TARGET_HTTPC_OBJS) $(TARGET_HTTPC_LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid httpc completed!'
	@echo ''
	@echo '################################################################'
				
$(TARGET_CAM_VIEWER): $(TARGET_CAM_VIEWER_OBJS)
	$(CXX) -o $(TARGET_CAM_VIEWER) $(TARGET_CAM_VIEWER_OBJS) $(LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid cam-viewer completed!'
	@echo ''
	@echo '################################################################'	
		
$(TARGET_CAM_PUSHER): $(TARGET_CAM_PUSHER_OBJS)
	$(CXX) -o $(TARGET_CAM_PUSHER) $(TARGET_CAM_PUSHER_OBJS) $(LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid cam-pusher completed!'
	@echo ''
	@echo '################################################################'	

$(TARGET_CAMCHAT_PUSHER): $(TARGET_CAMCHAT_PUSHER_OBJS)
	$(CXX) -o $(TARGET_CAMCHAT_PUSHER) $(TARGET_CAMCHAT_PUSHER_OBJS) $(LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid camchat-pusher completed!'
	@echo ''
	@echo '################################################################'
						
all: $(TARGET) $(TARGET_GLIB) $(TARGET_HTTPD) $(TARGET_HTTPC) $(TARGET_CAM_VIEWER) $(TARGET_CAM_PUSHER) $(TARGET_CAMCHAT_PUSHER) $(TARGET_LEAK)

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET_GLIB) \
	$(TARGET_HTTPD) $(TARGET_HTTPC) \
	$(TARGET_CAM_VIEWER) $(TARGET_CAM_VIEWER_OBJS) \
	$(TARGET_CAM_PUSHER) $(TARGET_CAM_PUSHER_OBJS) \
	$(TARGET_CAMCHAT_PUSHER) $(TARGET_CAMCHAT_PUSHER_OBJS) \
	$(TARGET_LEAK)