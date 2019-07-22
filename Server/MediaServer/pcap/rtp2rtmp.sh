#!/bin/sh
# Transcode RTP Stream to RTMP Stream script
# Author:	Max.Chiu

FFMPEG=/Users/max/Documents/tools/build/bin/ffmpeg

#$FFMPEG -re -i rtp.sdp -vcodec libx264 -preset superfast -profile:v baseline -level 3.0 -b:v 500k -threads 2 -c:a libfdk_aac -strict -2 -ar 44100 -ac 1 -f flv -y rtmp://172.25.32.17:19351/live/max0
#$FFMPEG -loglevel trace -protocol_whitelist "file,http,https,rtp,rtcp,udp,tcp,tls" -thread_queue_size 512 -re -i rtp.sdp -vcodec libx264 -profile:v baseline -level 3.0 -b:v 500k -c:a libfdk_aac -strict -2 -ar 44100 -ac 1 -f flv -y rtmp://172.25.32.17:19351/live/max0
$FFMPEG -protocol_whitelist "file,http,https,rtp,rtcp,udp,tcp,tls" -thread_queue_size 1024 -re -i rtp.sdp -vcodec libx264 -preset superfast -profile:v baseline -level 3.0 -b:v 300k -c:a libfdk_aac -strict -2 -ar 44100 -ac 1 -f flv rtmp://172.25.32.17:19351/live/max0