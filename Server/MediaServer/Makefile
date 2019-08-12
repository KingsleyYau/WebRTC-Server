# Copyright (C) 2019 The Qpidnetwork
# MediaServer Makefile
#
# Created on: 2019/6/13
# Author: Max.Chiu
# Email: Kingsleyyau@gmail.com
#

export MAKE	:=	make

release=0
ifeq ($(release), 1)
CXXFLAGS = -O3 
else
CXXFLAGS = -O2 -g #-D_CHECK_MEMORY_LEAK
endif

CXXFLAGS += -std=c++11
CXXFLAGS += -Wall -fmessage-length=0 -Wno-unused-variable -Wno-unused-but-set-variable
CXXFLAGS += -I. -Iinclude -Ithird_party \
			-Ithird_party/curl/include \
			-Ithird_party/libsdp/include \
			-Ithird_party/json -Ithird_party/websocketpp \
			-I$(OPENSSL_INCLUDE)

export BUILD_PATH = /root/webrtc/build
export OPENSSL_INCLUDE = $(BUILD_PATH)/include
export OPENSSL_LIB = $(BUILD_PATH)/lib

# 注意连接依赖顺序问题, 被依赖的放后面
LIBS = \
	-L. \
	-Wl,-Bstatic -Lwebrtc -lwebrtc \
	-Wl,-Bstatic -Lrtmp -lrtmp \
	-Wl,-Bstatic -Lrtp -lrtp \
	-Wl,-Bstatic -Lsocket -lsocket \
	-Wl,-Bstatic -Lmedia -lmedia \
	-Wl,-Bstatic -Lparser -lparser \
	-Wl,-Bstatic -Lrequest -lrequest \
	-Wl,-Bstatic -Lrespond -lrespond \
	-Wl,-Bstatic -Lhttpclient -lhttpclient \
	-Wl,-Bstatic -Lsimulatorchecker -lsimulatorchecker \
	-Wl,-Bstatic -Lwebsocket -lwebsocket \
	-Wl,-Bstatic -Lserver -lserver \
	-Wl,-Bstatic -Lcommon -lcommon \
	-Wl,-Bstatic -Lthird_party/libsrtp -lsrtp2 \
	-Wl,-Bstatic -Lthird_party/libsdp -lsdp \
	-Wl,-Bstatic -Lthird_party/amf -lamf \
	-Wl,-Bstatic -Lthird_party/json -ljson \
	-Wl,-Bstatic -Lthird_party/xml -lxml \
	-Wl,-Bstatic -Lthird_party/libev/.libs -lev \
	-Wl,-Bstatic -Lthird_party/esl/libs -lesl \
	-Wl,-Bstatic -Lthird_party/curl/lib/.libs -lcurl \
	-Wl,-Bstatic -Lthird_party/sqlite/.libs -lsqlite3 \
	-Wl,-Bstatic -Lthird_party/libnice/nice/.libs -lnice \
	-Wl,-Bstatic -Lthird_party/libnice/stun/.libs -lstun \
	-Wl,-Bstatic -Lthird_party/libnice/agent/.libs -lagent \
	-Wl,-Bstatic -Lthird_party/libnice/random/.libs -lnice-random \
	-Wl,-Bstatic -Lthird_party/libnice/socket/.libs -lsocket \
	-Wl,-Bdynamic -lgio-2.0 -lgobject-2.0 -lgthread-2.0 -lglib-2.0 \
	-Wl,-Bdynamic -lboost_chrono -lboost_system -lboost_random \
	-Wl,-Bstatic -L$(OPENSSL_LIB) -lssl -lcrypto \
	-Wl,-Bdynamic -ldl -pthread -lidn -lrt -lresolv -lselinux -lpcre
	
OBJS = main.o MediaServer.o
		
TARGET = media-server

MODULEDIRS = \
	common httpclient simulatorchecker \
	parser server \
	request respond \
	websocket \
	webrtc rtp rtmp socket \
	media 
			
CLEAN_MODULES = $(addprefix _clean_, $(MODULEDIRS))

DEPDIRS	= \
	third_party/libev third_party/sqlite third_party/curl \
	third_party/amf third_party/json third_party/xml \
	third_party/libsdp third_party/libsrtp third_party/libnice
			
CLEAN_DEPS = $(addprefix _clean_, $(DEPDIRS))

.PHONY: all deps modules test clean cleanall install $(DEPDIRS) $(MODULEDIRS) $(TARGET)

$(TARGET): deps modules $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid media-server completed!'
	@echo ''
	@echo '################################################################'

$(MODULEDIRS):
	$(MAKE) -C $@
	
$(CLEAN_MODULES):	
	$(MAKE) -C $(patsubst _clean_%, %, $@) clean
	
$(DEPDIRS):
	$(MAKE) -C $@
	
$(CLEAN_DEPS):	
	$(MAKE) -C $(patsubst _clean_%, %, $@) clean
					
deps: $(DEPDIRS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid dependence completed!'
	@echo ''
	@echo '################################################################'
	
modules: $(MODULEDIRS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid modules completed!'
	@echo ''
	@echo '################################################################'

all: deps modules $(TARGET)

test: deps modules

clean: $(CLEAN_MODULES)
	rm -f $(TARGET) $(OBJS)
	
cleanall: clean	$(CLEAN_DEPS)
	@echo '################################################################'
	@echo ''
	@echo '# Clean mediaserver all finished!'
	@echo ''
	@echo '################################################################'
	
install: 
	copy media-server.config /etc/ -rf
	copy media-server /usr/local/bin
	chmod +x /usr/local/bin/mediaserver
	@echo '################################################################'
	@echo ''
	@echo '# Install mediaserver finished!'
	@echo ''
	@echo '################################################################'
