#!/bin/sh
# Watch mediaserver script
# Author: Max.Chiu
# Date: 2019/08/13

watch -n 1 -c "ps -efww | grep -v tail | grep -v grep | grep -v 'js\|ffmpeg\|start_\|check_status' | grep 'mediaserver\|rtmp2rtp\|rtp2rtmp';\
echo "-------------------- 		top		 --------------------";\
top -b -n 1 | head -n 6;\
top -b -n 1 | grep 'mediaserver\|turnserver\|ffmpeg';\
echo "-------------------- 	netstat	 --------------------";\
echo "Listen Socket:" && netstat -nplt | grep 'mediaserver\|turnserver';\
echo "";\
echo "TCP Socket:" && netstat -anplt | grep 'mediaserver\|turnserver\|ffmpeg' | wc -l;\
echo "UDP Socket:" && netstat -anpu | grep 'mediaserver\|turnserver\|ffmpeg' | wc -l;\
echo "-------------------- 	sar	 --------------------";\
echo "Network:" && sar -n DEV 1 1 | grep Average:\
"