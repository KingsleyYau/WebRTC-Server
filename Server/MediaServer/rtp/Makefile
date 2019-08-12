# Copyright (C) 2019 The Qpidnetwork
# Rtp Module Makefile
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

CXXFLAGS 	+=	-DLINUX -DFILE_JNI_LOG
CXXFLAGS 	+=	-Wall -fmessage-length=0 -Wno-unused-variable -Wno-unused-but-set-variable -D__STDC_CONSTANT_MACROS
CXXFLAGS 	+=	-I. -I.. -I../third_party -I../third_party/libsrtp/include -I../third_party/libsrtp/crypto/include \
				-I$(OPENSSL_INCLUDE)
				
OBJS 		+=	DtlsSession.o RtpSession.o RtpRawClient.o
		
TARGET 		=	librtp.a

.PHONY: all clean $(TARGET)

$(TARGET):	$(OBJS)
	$(AR) cr $@ $(OBJS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid librtp.a completed!'
	@echo ''
	@echo '################################################################'

all:	$(TARGET) 

clean:
	rm -f $(OBJS) $(TARGET)