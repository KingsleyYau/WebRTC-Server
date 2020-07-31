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
				-I../third_party/abseil-cpp -I../../third_party/abseil-cpp -I../../../third_party/abseil-cpp \
				-I$(OPENSSL_INCLUDE)
				
OBJS		+=	api/units/timestamp.o \
				api/media_types.o api/rtp_parameters.o api/rtp_headers.o
OBJS		+=	base/ntp_time.o base/rw_lock_wrapper.o base/rw_lock_posix.o base/clock.o base/time_utils.o \
				base/string_encode.o base/string_to_number.o base/strings/string_builder.o 
OBJS		+=	include/rtp_rtcp_defines.o include/rtp_header_extension_map.o include/rtp_header_extension_size.o \
				include/rtp_header_extensions.o include/time_util.o 
OBJS		+=	packet/RtpPacket.o packet/rtp_packet_received.o \
				packet/RtcpPacket.o packet/CommonHeader.o \
				packet/Dlrr.o packet/Rrtr.o packet/TargetBitrate.o packet/ExtendedReports.o \
				packet/ReportBlock.o packet/SenderReport.o \
				packet/receiver_report.o \
				packet/sdes.o \
				packet/Psfb.o packet/Pli.o packet/Fir.o packet/Nack.o packet/Remb.o \
				packet/Rtpfb.o packet/TransportFeedback.o
OBJS		+=	module/RemoteEstimatorProxy.o
OBJS		+=	DtlsSession.o RtpSession.o RtpRawClient.o
						
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