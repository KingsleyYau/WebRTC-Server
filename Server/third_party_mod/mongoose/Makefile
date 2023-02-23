# Copyright (C) 2020 The Qpidnetwork
# mongoose module Makefile
#
# Created on: 2020/11/10
# Author: Max.Chiu
# Email: Kingsleyyau@gmail.com
#

export MAKE	:=	make

release=0
ifeq ($(release), 1)
CFLAGS = -O3 
else
CFLAGS = -g
endif

CFLAGS += -Wall -fmessage-length=0 -Wno-unused-variable -Wno-unused-but-set-variable \
          -DDISABLE_MD5=1 -DMG_ENABLE_HTTP_CGI=0 #\
          #-DCS_ENABLE_STDIO=1 -DCS_ENABLE_DEBUG=1
CFLAGS += -I. -I.. -I../include

OBJS = mongoose.o
		
TARGET = libmongoose.a

.PHONY: all clean $(TARGET)

$(TARGET):	$(OBJS)
	$(AR) cr $@ $(OBJS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid libmongoose.a completed!'
	@echo ''
	@echo '################################################################'	
	
all:	$(TARGET) 

clean:
	rm -f $(OBJS) $(TARGET)