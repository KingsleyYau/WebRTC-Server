#!/bin/sh
# Watch mediaserver script
# Author:	Max.Chiu

watch -n 1 -c "ps -ef | grep -v tail | grep -v grep | grep -v js | grep -v ffmpeg | grep 'mediaserver' && \
echo "-------------------- 		top		 --------------------" && \
top -b -n 1 | head -n 6 && \
top -b -n 1 | grep 'mediaserver\|turnserver\|ffmpeg' && \
echo "-------------------- 	netstat	 --------------------" && \
echo "TCP Open Files:" && netstat -anplt | grep 'mediaserver\|turnserver\|ffmpeg' | wc -l && \
echo "UDP Open Files:" && netstat -anpu | grep 'mediaserver\|turnserver\|ffmpeg' | wc -l \
"