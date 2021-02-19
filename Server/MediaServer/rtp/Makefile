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

CXXFLAGS 	+=	-std=c++11 -DLINUX -DFILE_JNI_LOG -DWEBRTC_POSIX -DWEBRTC_LINUX
CXXFLAGS 	+=	-Wall -fmessage-length=0 -Wno-unused-variable -Wno-unused-but-set-variable -D__STDC_CONSTANT_MACROS
CXXFLAGS 	+=	-I. -I.. -I../third_party -I../third_party/libsrtp/include -I../third_party/libsrtp/crypto/include \
				-I../third_party/abseil-cpp -I../../third_party/abseil-cpp -I../../../third_party/abseil-cpp \
				-I$(BUILD_PATH_INCLUDE)
				
OBJS		+=  api/transport/network_types.o api/transport/field_trial_based_config.o \
				api/units/data_rate.o api/units/data_size.o api/units/frequency.o api/units/timestamp.o \
				api/media_types.o api/rtp_parameters.o api/rtp_headers.o \
				api/video/video_timing.o
				
OBJS		+=	base/ntp_time.o base/rw_lock_wrapper.o base/rw_lock_posix.o base/clock.o base/time_utils.o \
				base/string_encode.o base/string_to_number.o \
				base/metrics.o base/field_trial.o \
				base/platform_thread_types.o base/critical_section.o \
				base/race_checker.o base/rate_statistics.o \
				base/strings/string_builder.o \
				base/experiments/field_trial_parser.o \
				base/zero_memory.o base/copy_on_write_buffer.o 
				
OBJS		+=	include/rtp_rtcp_defines.o include/rtp_header_extension_map.o include/rtp_header_extension_size.o \
				include/rtp_header_extensions.o include/time_util.o \
				include/receive_statistics_impl.o include/rtp_packet_history.o
				
OBJS		+=	packet/rtp_packet.o packet/rtp_packet_received.o packet/rtp_packet_to_send.o \
				packet/RtcpPacket.o packet/CommonHeader.o \
				packet/Dlrr.o packet/Rrtr.o packet/TargetBitrate.o packet/ExtendedReports.o \
				packet/ReportBlock.o packet/SenderReport.o \
				packet/receiver_report.o \
				packet/sdes.o \
				packet/Psfb.o packet/Pli.o packet/Fir.o packet/Nack.o packet/Remb.o \
				packet/Rtpfb.o packet/TransportFeedback.o \
				packet/app.o packet/remote_estimate.o \
				
OBJS		+=	modules/nack_module.o modules/nack_audio_module.o \
				modules/congestion_controller/goog_cc/link_capacity_estimator.o \
				modules/remote_bitrate_estimator/bwe_defines.o \
				modules/remote_bitrate_estimator/aimd_rate_control.o \
				modules/remote_bitrate_estimator/overuse_detector.o \
				modules/remote_bitrate_estimator/overuse_estimator.o \
				modules/remote_bitrate_estimator/inter_arrival.o \
				modules/remote_bitrate_estimator/remote_bitrate_estimator_abs_send_time.o 
				
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