#!/bin/sh
# Watch mediaserver script
# Author: Max.Chiu
# Date: 2019/08/13

watch -n 1 -c "ps -ef | grep -v tail | grep -v grep | grep -v js | grep -v ffmpeg | grep 'mediaserver';\
echo "-------------------- 		top		 --------------------";\
top -b -n 1 | head -n 6;\
top -b -n 1 | grep 'mediaserver\|turnserver\|ffmpeg' | grep -v 0.0;\
echo "-------------------- 	netstat	 --------------------";\
echo "TCP Socket:" && netstat -anplt | grep 'mediaserver\|turnserver\|ffmpeg'\
echo "UDP Socket:" && netstat -anpu | grep 'mediaserver\|turnserver\|ffmpeg'\
echo "-------------------- 	sar	 --------------------";\
echo "Network:" && sar -n DEV 1 1 | grep Average:\
"