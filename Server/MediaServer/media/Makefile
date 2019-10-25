# Copyright (C) 2019 The Qpidnetwork
# Media Module Makefile
#
# Created on: 2019/6/13
# Author: Max.Chiu
# Email: Kingsleyyau@gmail.com
#

export MAKE	:=	make

ifeq ($(RELEASE), 1)
CXXFLAGS = -O2 
else
CXXFLAGS = -O2 -g
endif

CXXFLAGS 	+=	-Wall -fmessage-length=0 -Wno-unused-variable -Wno-unused-but-set-variable
CXXFLAGS 	+=	-I. -I..

OBJS 		=	$(THIRD_PARTY)
OBJS 		+=	VideoMuxer.o
		
TARGET 		=	libmedia.a

.PHONY: all clean $(TARGET)

$(TARGET):	$(OBJS)
	$(AR) cr $@ $(OBJS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid libmedia.a completed!'
	@echo ''
	@echo '################################################################'

all:	$(TARGET) 

clean:
	rm -f $(OBJS) $(TARGET)