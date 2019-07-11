# Copyright (C) 2015 The CamShareMiddleware
# tester Makefile
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

CXXFLAGS +=	-Wall -fmessage-length=0 -Wunused-variable -Wno-write-strings -Wno-narrowing -Wno-unused-variable
CXXFLAGS +=	-I. -I../ -I../include -I../third_party -I../third_party/libev

CXXFLAGS += -D__STDC_CONSTANT_MACROS

# 注意连接依赖顺序问题, 被依赖的放后面
LIBS =		-L. \
			-Wl,-Bstatic -L../common -lcommon \
			-Wl,-Bstatic -L../third_party/libev/.libs -lev \
			-Wl,-Bdynamic -ldl -lz -lpthread -lidn
				
OBJS	+=	http_client.o HttpServerTest.o		
TARGET =	http-client

FORK_OBJS += $(COMMON)
FORK_OBJS += fork_tester.o
FORK_TEST =	fork-tester

TRANSCODER_OBJS += $(COMMON)
TRANSCODER_OBJS += transcoder_tester.o
TRANSCODER_TEST =	transcoder-tester

.PHONY: clean $(TARGET) $(FORK_TEST) $(TRANSCODER_TEST)

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid tester completed!'
	@echo ''
	@echo '################################################################'

$(FORK_TEST):	$(FORK_OBJS)
	$(CXX) -o $(FORK_TEST) $(FORK_OBJS) $(LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid fork-tester completed!'
	@echo ''
	@echo '################################################################'
	
$(TRANSCODER_TEST):	$(TRANSCODER_OBJS)
	$(CXX) -o $(TRANSCODER_TEST) $(TRANSCODER_OBJS) $(LIBS)
	@echo '################################################################'
	@echo ''
	@echo '# Bulid transcoder-tester completed!'
	@echo ''
	@echo '################################################################'
		
all:	$(TARGET) $(FORK_TEST) $(TRANSCODER_TEST)

clean:
	rm -f $(OBJS) $(TARGET) \
	$(FORK_OBJS) $(FORK_TEST) \
	$(TRANSCODER_TEST) $(TRANSCODER_OBJS)