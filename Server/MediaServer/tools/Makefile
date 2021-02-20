# Copyright (C) 2020 The Mediaserver
# webrtc-tester Makefile
#
# Created on: 2020/11/10
# Author: Max.Chiu
# Email: Kingsleyyau@gmail.com
#

export MAKE	:=	make

ifeq ($(RELEASE), 1)
CXXFLAGS = -O3 
else
CXXFLAGS = -O2 -g
endif

CXXFLAGS += -std=c++11
CXXFLAGS +=	-Wall -fmessage-length=0 -Wunused-variable -Wno-write-strings -Wno-narrowing -Wno-unused-variable -fpermissive
CXXFLAGS +=	-I. -I../ -I../include -I../third_party \
			-I../third_party/json \
			-I$(BUILD_PATH_INCLUDE)

CFLAGS += -DDISABLE_MD5=1

# 注意连接依赖顺序问题, 被依赖的放后面
LIBS = -L. \
		-Wl,-Bstatic -L../common -lcommon \
		-Wl,-Bstatic -L../third_party/json -ljson \
		-Wl,-Bstatic -L../third_party/mongoose -lmongoose \
		-Wl,-Bstatic -L$(BUILD_PATH_LIB) -lssl -lcrypto \
		-Wl,-Bdynamic -ldl -pthread 
OBJS = wscat.o
TARGET = wscat

# 注意连接依赖顺序问题, 被依赖的放后面
WSSTRESS_LIBS = -L. \
		-Wl,-Bstatic -L../common -lcommon \
		-Wl,-Bstatic -L../third_party/json -ljson \
		-Wl,-Bstatic -L../third_party/mongoose -lmongoose \
		-Wl,-Bstatic -L$(BUILD_PATH_LIB) -lssl -lcrypto \
		-Wl,-Bdynamic -ldl -pthread 
WSSTRESS_OBJS = wsstress.o
WSSTRESS = wsstress

# 注意连接依赖顺序问题, 被依赖的放后面
SNAPSHOT_LIBS =	-L. \
    -Wl,-Bstatic -L../common -lcommon \
    -Wl,-Bstatic -L$(BUILD_PATH_LIB) -lavfilter -lavformat -lavdevice -lavcodec -lswscale -lavutil -lswresample -lpostproc \
    -lx264 -lfdk-aac -lopus -lssl -lcrypto \
    -Wl,-Bdynamic -ldl -lz -lpthread -lidn -lrt
SNAPSHOT_OBJS = snapshot.o
SNAPSHOT = snapshot
			
.PHONY: clean $(TARGET) $(SNAPSHOT)
	
$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid wscat completed!'
	@echo ''
	@echo '################################################################'

$(WSSTRESS): $(WSSTRESS_OBJS)
	$(CXX) -o $(WSSTRESS) $(WSSTRESS_OBJS) $(WSSTRESS_LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid wsstress completed!'
	@echo ''
	@echo '################################################################'
	
$(SNAPSHOT) : $(SNAPSHOT_OBJS)
	$(CXX) -o $(SNAPSHOT) $(SNAPSHOT_OBJS) $(SNAPSHOT_LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid snapshot completed!'
	@echo ''
	@echo '################################################################'
	
all: $(TARGET) $(SNAPSHOT)

clean:
	rm -f $(OBJS) $(TARGET) $(WSSTRESS) $(SNAPSHOT)