# Copyright (C) 2019 The Qpidnetwork
# Session Description Protocol library Makefile
#
# Created on: 2019/6/13
# Author: Max.Chiu
# Email: Kingsleyyau@gmail.com
#

export MAKE	:=	make

release=0
ifeq ($(release), 1)
CXXFLAGS = -O3 
CFLAGS = -O3
else
CXXFLAGS = -O2 -g
CFLAGS = -O2 -g
endif

CFLAGS 		+=	-I. -I./include -DSDP_API_EXPORTS -fvisibility=hidden -std=gnu99
CXXFLAGS 	+=	-DLINUX -DFILE_JNI_LOG
CXXFLAGS 	+=	-Wall -fmessage-length=0 -Wno-unused-variable -Wno-unused-but-set-variable -D__STDC_CONSTANT_MACROS
CXXFLAGS 	+=	-I. -I./include

OBJS 		=	$(THIRD_PARTY)
OBJS 		+=	src/sdp_base64.o src/sdp.o src/ulog.o
		
TARGET 		=	libsdp.a

.PHONY: all clean $(TARGET)

$(TARGET):	$(OBJS)
	$(AR) cr $@ $(OBJS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid libsdp.a completed!'
	@echo ''
	@echo '################################################################'

all:	$(TARGET) 

clean:
	rm -f $(OBJS) $(TARGET)