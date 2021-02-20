# Copyright (C) 2020 The Mediaserver
# wsserver-tester Makefile
#
# Created on: 2020/11/10
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
CXXFLAGS +=	-Wall -fmessage-length=0 -Wunused-variable -Wno-write-strings -Wno-narrowing -Wno-unused-variable -fpermissive
CXXFLAGS +=	-I. -I../../ -I../../include -I../../third_party \
			-I../../third_party/json \
			-I../../third_party/websocketpp \
			-I$(BUILD_PATH_INCLUDE)

CFLAGS += -DDISABLE_MD5=1

# 注意连接依赖顺序问题, 被依赖的放后面
LIBS = -L. \
        -Wl,-Bstatic -L../ -lwebsocket \
		-Wl,-Bstatic -L../../common -lcommon \
		-Wl,-Bstatic -L../../third_party/json -ljson \
		-Wl,-Bstatic -L$(BUILD_PATH_LIB) -lssl -lcrypto \
		-Wl,-Bdynamic -lboost_chrono -lboost_system -lboost_random \
		-Wl,-Bdynamic -ldl -pthread 
OBJS = wsserver_tester.o
TARGET = wsserver_tester
			
.PHONY: clean $(TARGET)
	
$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid wsserver_tester completed!'
	@echo ''
	@echo '################################################################'

#############################'

all: $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)