#!/bin/sh
# Transcode RTP Stream to RTMP Stream script
# Author:	Max.Chiu

FFMPEG=/Users/max/Documents/tools/build/bin/ffmpeg

#$FFMPEG -re -i rtp.sdp -vcodec libx264 -preset superfast -profile:v baseline -level 3.0 -b:v 500k -c:a libfdk_aac -strict -2 -ar 44100 -ac 1 -f flv -y rtmp://172.25.32.17:19351/live/max0
$FFMPEG -re -i rtp.sdp -vcodec copy -c:a libfdk_aac -strict -2 -ar 44100 -ac 1 -f flv -y rtmp://172.25.32.17:19351/live/max0