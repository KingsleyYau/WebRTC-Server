# Copyright (C) 2019 The Qpidnetwork
# Rtp Module Makefile
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

CXXFLAGS 	+=	-std=c++11 -DLINUX -DFILE_JNI_LOG 
CXXFLAGS 	+=	-Wall -fmessage-length=0 -Wno-unused-variable -Wno-unused-but-set-variable -D__STDC_CONSTANT_MACROS
CXXFLAGS 	+=	-I. -I.. -I../third_party -I../third_party/libsrtp/include -I../third_party/libsrtp/crypto/include \
				-I../third_party/abseil-cpp -I../../third_party/abseil-cpp \
				-I$(OPENSSL_INCLUDE)
				
OBJS		+=	DtlsSession.o RtpSession.o RtpRawClient.o NtpTime.o
OBJS		+=	packet/RtpPacketImp.o  packet/RtcpPacketImp.o packet/CommonHeader.o \
				packet/Psfb.o packet/Pli.o packet/Fir.o packet/Remb.o \
				packet/Rtpfb.o packet/Nack.o packet/TransportFeedback.o 
		
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