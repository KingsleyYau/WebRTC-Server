#!/bin/sh
# Watch mediaserver script
# Author: Max.Chiu
# Date: 2019/08/13

watch -n 3 -c "ps -efww | grep -v tail | grep -v grep | grep 'mediaserver' | grep -v 'js\|ffmpeg\|start_\|check_status\|rtmp2rtp\|rtp2rtmp' | sort -k 8;\
echo "-------------------- 	netstat	 --------------------";\
echo "Listen Socket:" && netstat -nplt 2>/dev/null | grep '3478\|5766\|9880\|9881\|9081\|9980\|9981\|9082\|9883\|9083';\
echo "";\
echo "WS:" && netstat -anpt 2>/dev/null | grep '9881\|9981\|9883' | grep 'ESTABLISHED' | wc -l ;\
echo "TCP Socket:" && netstat -anpt 2>/dev/null | grep 'turnserver' | grep 'ESTABLISHED' | wc -l;\
echo "UDP Socket:" && netstat -anpu 2>/dev/null | grep 'mediaserver\|turnserver' | wc -l;\
echo "FFMPEG Socket:" && netstat -anptu 2>/dev/null | grep 'ffmpeg' | wc -l;\
echo "TIME_WAIT:" && netstat -antlpu 2>/dev/null | grep -c 'TIME_WAIT';\
echo "-------------------- 	sar	 --------------------";\
echo "Network:" && sar -n DEV 1 1 | grep Average:;\
echo "-------------------- 		top		 --------------------";\
top -b -n 3 | grep -A 4 'load average' | tail -n 5;\
top -b -n 1 | grep 'turnserver\|mediaserver';\
"