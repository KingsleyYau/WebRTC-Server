# Copyright (C) 2015 The CamShareServer
# amf module Makefile
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
CXXFLAGS = -O2 -g
endif

CXXFLAGS +=	-Wall -fmessage-length=0 -Wno-unused-variable -Wno-unused-but-set-variable
CXXFLAGS +=	-I. -I.. -I../include

OBJS 		=	AMF3.o AmfParser.o
		
TARGET =	libamf.a

.PHONY: all clean $(TARGET)

$(TARGET):	$(OBJS)
	$(AR) cr $@ $(OBJS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid libamf.a completed!'
	@echo ''
	@echo '################################################################'	
	
all:	$(TARGET) 

clean:
	rm -f $(OBJS) $(TARGET)