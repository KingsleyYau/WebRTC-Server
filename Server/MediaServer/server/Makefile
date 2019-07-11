# Copyright (C) 2015 The MediaServer
# server module Makefile
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

CXXFLAGS +=	-Wall -fmessage-length=0 -Wno-unused-variable -Wno-unused-but-set-variable
CXXFLAGS +=	-I. -I.. -I../include -I../third_party/libev

OBJS 		=	TcpServer.o AsyncIOServer.o
		
TARGET =	libserver.a

.PHONY: all deps $(TARGET)

$(TARGET):	$(OBJS)
	$(AR) cr $@ $(OBJS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid libserver.a completed!'
	@echo ''
	@echo '################################################################'
	
all:	$(TARGET) 

clean:
	rm -f $(OBJS) $(TARGET)
