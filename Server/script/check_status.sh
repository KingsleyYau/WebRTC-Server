#!/bin/sh
# Watch mediaserver script
# Author: Max.Chiu
# Date: 2019/08/13

watch -n 1 -c "ps -efww | grep -v tail | grep -v grep | grep -v 'js\|ffmpeg\|start_\|check_status' | grep 'mediaserver\|rtmp2rtp\|rtp2rtmp';\
echo "-------------------- 	netstat	 --------------------";\
echo "Listen Socket:" && netstat -nplt 2>/dev/null | grep '3478\|5766\|9880\|9881\|9980\|9981';\
echo "";\
echo "TCP Socket:" && netstat -anpt 2>/dev/null | grep 'turnserver' | wc -l;\
echo "UDP Socket:" && netstat -anpu 2>/dev/null | grep 'mediaserver\|turnserver' | wc -l;\
echo "FFMPEG Socket:" && netstat -anptu 2>/dev/null | grep 'ffmpeg' | wc -l;\
echo "TIME_WAIT:" && netstat -antlpu 2>/dev/null | grep -c 'TIME_WAIT';\
echo "-------------------- 	sar	 --------------------";\
echo "Network:" && sar -n DEV 1 1 | grep Average:;\
echo "-------------------- 		top		 --------------------";\
top -b -n 1 | head -n 6;\
top -b -n 1 | grep 'turnserver\|mediaserver';\
"