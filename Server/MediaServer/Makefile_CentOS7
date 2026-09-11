# Copyright (C) 2019 The Mediaserver
# MediaServer Makefile
#
# Created on: 2019/6/13
# Author: Max.Chiu
# Email: Kingsleyyau@gmail.com
#

export MAKE	:=	make

export RELEASE = 0
ifeq ($(RELEASE), 1)
CXXFLAGS = -O2
else
CXXFLAGS = -O2 -g #-D_CHECK_MEMORY_LEAK
endif

CXXFLAGS += -std=c++11 -DWEBRTC_POSIX -DWEBRTC_LINUX
CXXFLAGS += -Wall -fmessage-length=0 -Wno-unused-variable -Wno-unused-but-set-variable
CXXFLAGS += -I. -Iinclude -Ithird_party \
			-Ithird_party/curl/include \
			-Ithird_party/libsdp/include \
			-Ithird_party/json \
			-Ithird_party/websocketpp \
			-Ithird_party/abseil-cpp \
			-I/usr/include/glib-2.0/ -I/usr/lib64/glib-2.0/include/ \
			-I$(BUILD_PATH_INCLUDE)

export BUILD_PATH = /root/mediaserver/build
export BUILD_PATH_INCLUDE = $(BUILD_PATH)/include
export BUILD_PATH_LIB = $(BUILD_PATH)/lib

# 注意连接依赖顺序问题, 被依赖的放后面
LIBS = \
	-L. \
	-Wl,-Bstatic -Lwebrtc -lwebrtc \
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
	-Wl,-Bstatic -Lcrypto -lmediacrypto \
	-Wl,-Bstatic -Lthird_party/libsrtp -lsrtp2 \
	-Wl,-Bstatic -Lthird_party/libsdp -lsdp \
	-Wl,-Bstatic -Lthird_party/amf -lamf \
	-Wl,-Bstatic -Lthird_party/json -ljson \
	-Wl,-Bstatic -Lthird_party/xml -lxml \
	-Wl,-Bstatic -Lthird_party/libev/.libs -lev \
	-Wl,-Bstatic -Lthird_party/curl/lib/.libs -lcurl \
	-Wl,-Bstatic -Lthird_party/sqlite/.libs -lsqlite3 \
	-Wl,-Bstatic -Lthird_party/libnice/nice/.libs -lnice \
	-Wl,-Bstatic -Lthird_party/libnice/stun/.libs -lstun \
	-Wl,-Bstatic -Lthird_party/libnice/agent/.libs -lagent \
	-Wl,-Bstatic -Lthird_party/libnice/random/.libs -lnice-random \
	-Wl,-Bstatic -Lthird_party/libnice/socket/.libs -lsocket \
	-Wl,-Bstatic -Lthird_party/abseil-cpp/absl/types -labsl_bad_optional_access \
	-Wl,-Bstatic -Lthird_party/abseil-cpp/absl/types -labsl_bad_variant_access \
	-Wl,-Bstatic -Lthird_party/abseil-cpp/absl/types -labsl_bad_any_cast_impl \
	-Wl,-Bdynamic -lgio-2.0 -lgobject-2.0 -lgthread-2.0 -lglib-2.0 \
	-Wl,-Bdynamic -lboost_chrono -lboost_system -lboost_random \
	-Wl,-Bstatic -L$(BUILD_PATH_LIB) -lssl -lcrypto \
	-Wl,-Bdynamic -ldl -pthread -lidn -lrt -lresolv -lselinux -lpcre -luuid -lz
	
OBJS = main.o MediaServer.o
		
TARGET = mediaserver

MODULEDIRS = \
	common crypto httpclient simulatorchecker \
	parser server \
	request respond \
	websocket \
	webrtc rtp socket \
	media tools
			
CLEAN_MODULES = $(addprefix _clean_, $(MODULEDIRS))

DEPDIRS	= \
	third_party/libev third_party/sqlite third_party/curl \
	third_party/amf third_party/json third_party/xml \
	third_party/libsdp third_party/libsrtp third_party/libnice \
	third_party/abseil-cpp third_party/mongoose
			
CLEAN_DEPS = $(addprefix _clean_, $(DEPDIRS))

.PHONY: all version deps modules test clean cleanall install $(DEPDIRS) $(MODULEDIRS) $(TARGET)

$(TARGET): version deps modules $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid mediaserver completed!'
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
	
version:
	rm -f main.o
	
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

all: deps modules $(TARGET) test

test: deps modules
	$(MAKE) -C test

clean: $(CLEAN_MODULES)
	rm -f $(TARGET) $(OBJS)
	
cleanall: clean	$(CLEAN_DEPS)
	@echo '################################################################'
	@echo ''
	@echo '# Clean mediaserver all finished!'
	@echo ''
	@echo '################################################################'
	
install: 
	copy etc/mediaserver.config /etc/ -rf
	copy mediaserver /usr/local/bin
	chmod +x /usr/local/bin/mediaserver
	@echo '################################################################'
	@echo ''
	@echo '# Install mediaserver finished!'
	@echo ''
	@echo '################################################################'
