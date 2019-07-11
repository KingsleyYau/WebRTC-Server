# Copyright (C) 2019 The Qpidnetwork
# Rtmp Module Makefile
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
CXXFLAGS = -O2 -g
endif

CXXFLAGS 	+=	-DLINUX
CXXFLAGS 	+=	-Wall -fmessage-length=0 -Wno-unused-variable -Wno-unused-but-set-variable -D__STDC_CONSTANT_MACROS
CXXFLAGS 	+=	-I. -I.. -I../third_party -I../third_party/libev

THIRD_PARTY	=	../third_party/srs/srs_librtmp.o
				
OBJS 		=	$(THIRD_PARTY)
OBJS 		+=	RtmpStreamPool.o RtmpClient.o
		
TARGET 		=	librtmp.a

.PHONY: all clean $(TARGET)

$(TARGET):	$(OBJS)
	$(AR) cr $@ $(OBJS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid librtmp.a completed!'
	@echo ''
	@echo '################################################################'

all:	$(TARGET) 

clean:
	rm -f $(OBJS) $(TARGET)