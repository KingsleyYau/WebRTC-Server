#!/bin/sh
# Watch mediaserver script
# Author:	Max.Chiu

watch -n 1 -c "ps -ef | grep -v tail | grep -v grep | grep -v js | grep 'mediaserver\|ffmpeg' && netstat -anpu | grep -v 'freeswitch\|-'"