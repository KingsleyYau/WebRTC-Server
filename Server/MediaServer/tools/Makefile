# Copyright (C) 2020 The Mediaserver
# webrtc-tester Makefile
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
CXXFLAGS +=	-Wall -fmessage-length=0 -Wunused-variable -Wno-write-strings -Wno-narrowing -Wno-unused-variable 
CXXFLAGS +=	-I. -I../ -I../include -I../third_party \
			-I../third_party/json \
			-I$(OPENSSL_INCLUDE)

CFLAGS += -DDISABLE_MD5=1

# 注意连接依赖顺序问题, 被依赖的放后面
LIBS = -L. \
		-Wl,-Bstatic -L../common -lcommon \
		-Wl,-Bstatic -L../third_party/json -ljson \
		-Wl,-Bstatic -L../third_party/mongoose -lmongoose \
		-Wl,-Bstatic -L$(OPENSSL_LIB) -lssl -lcrypto \
		-Wl,-Bdynamic -ldl -pthread 
OBJS = wscat.o
TARGET = wscat

.PHONY: clean $(TARGET)
	
$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid wscat completed!'
	@echo ''
	@echo '################################################################'
	
all: $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)