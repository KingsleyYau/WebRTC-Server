# Copyright (C) 2015 The CamShareServer
# httpclient module Makefile
#
# Created on: 2015/10/10
# Author: Max.Chiu
# Email: Kingsleyyau@gmail.com
#

export MAKE	:=	make

ifeq ($(RELEASE), 1)
CXXFLAGS = -O2
else
CXXFLAGS = -O2 -g
endif

CXXFLAGS +=	-Wall -fmessage-length=0 -Wno-unused-variable -Wno-unused-but-set-variable
CXXFLAGS +=	-I. -I.. -I../include -I../third_party -I../third_party/curl/include

OBJS 		=	HttpClient.o HttpEntiy.o
		
TARGET =	libhttpclient.a

.PHONY: all clean $(TARGET)

$(TARGET):	$(OBJS)
	$(AR) cr $@ $(OBJS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid libhttpclient.a completed!'
	@echo ''
	@echo '################################################################'	
	
all:	$(TARGET) 

clean:
	rm -f $(OBJS) $(TARGET)