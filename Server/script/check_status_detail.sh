#!/bin/sh
# Watch mediaserver script
# Author: Max.Chiu
# Date: 2019/08/13

watch -n 1 -c "ps -efww | grep -v tail | grep -v grep | grep -v 'js\|ffmpeg\|start_\|check_status' | grep 'mediaserver\|rtmp2rtp\|rtp2rtmp';\
echo "-------------------- 	netstat	 --------------------";\
echo "TCP Socket:" && netstat -anpt 2>/dev/null | grep 'turnserver';\
echo "UDP Socket:" && netstat -anpu 2>/dev/null | grep -v 3478 | grep 'mediaserver\|turnserver';\
echo "FFMPEG Socket:" && netstat -anptu 2>/dev/null | grep 'ffmpeg';\
echo "TIME_WAIT:" && netstat -antlpu 2>/dev/null | grep -c 'TIME_WAIT';\
echo "-------------------- 	sar	 --------------------";\
echo "Network:" && sar -n DEV 1 1 | grep 'Average:';\
echo "-------------------- 		top		 --------------------";\
top -b -n 1 | head -n 6;\
top -b -n 1 | grep 'mediaserver\|turnserver\|ffmpeg' ;\
"