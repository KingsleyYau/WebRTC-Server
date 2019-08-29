#!/bin/sh
# RTP Stream to RTMP Stream script
# Author:	Max.Chiu

FFMPEG=/Users/max/Documents/tools/build/bin/ffmpeg

function Usage {
	echo "Usage : ./rtp2rtmp.sh rtp.sdp rtmp://192.168.88.17:19351/live/max0"
}

SDP_FILE=""
if [ ! "$1" == "" ]
then
	SDP_FILE=$1
else
	Usage;
	exit 1;
fi

RTMP_URL=""
if [ ! "$2" == "" ]
then
	RTMP_URL=$2
else
	Usage;
	exit 1;
fi

$FFMPEG -v trace -probesize 90000 -protocol_whitelist "file,http,https,rtp,rtcp,udp,tcp,tls" -thread_queue_size 1024 -re -i $SDP_FILE -vcodec libx264 -preset superfast -profile:v baseline -level 3.0 -g 12 -b:v 1000k -c:a libfdk_aac -strict -2 -ar 44100 -ac 1 -f flv $RTMP_URL