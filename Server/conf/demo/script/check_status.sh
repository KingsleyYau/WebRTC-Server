#!/bin/sh
# Watch mediaserver script
# Author:	Max.Chiu

watch -n 1 -c "ps -ef | grep -v tail | grep -v grep | grep -v js | grep 'mediaserver' && \
echo "-------------------- 		top		 --------------------" && \
top -b -n 1 | head -n 6 && \
top -b -n 1 | grep 'mediaserver\|turnserver\|ffmpeg' && \
echo "-------------------- 	netstat	 --------------------" && \
netstat -anpu | grep 'mediaserver\|turnserver\|ffmpeg' \
"