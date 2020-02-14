#!/bin/sh
# RTP stream to RTMP stream script
# Author: Max.Chiu
# Date: 2019/08/13

APP_DIR=$(dirname $(readlink -f "$0"))/..
FFMPEG=$APP_DIR/bin/ffmpeg

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

TRANSCODE=1
if [ ! "$3" == "" ]
then
	TRANSCODE=$3
fi

LOG_FILE=$SDP_FILE.log
if [ ! "$4" == "" ]
then
	LOG_FILE=$4
fi

trap 'Clean; exit' SIGTERM
function Clean() {
	SELF_PID=$$
	FFMPEG_PID=`ps --ppid $SELF_PID | grep ffmpeg | awk '{if($1~/[0-9]+/) print $1}'`
	if [ ! "$FFMPEG_PID" == "" ];then
		kill -9 $FFMPEG_PID
	fi
}

if [ "$TRANSCODE" -eq "1" ]
#if [ "1" -eq "1" ]
then
	$FFMPEG -probesize 90000 -protocol_whitelist "file,http,https,rtp,rtcp,udp,tcp,tls" \
					-v error \
					-thread_queue_size 1024 
					-i $SDP_FILE \
					-vcodec libx264 -preset superfast -profile:v baseline -level 3.0 -g 12 \
					-acodec:a libfdk_aac -strict -2 -ar 44100 -ac 1 \
					-f flv $RTMP_URL \
					>$LOG_FILE 2>&1 &
else
	$FFMPEG -probesize 90000 -protocol_whitelist "file,http,https,rtp,rtcp,udp,tcp,tls" \
					-v error \
					-thread_queue_size 1024 \
					-i $SDP_FILE \
					-vcodec copy \
					-acodec libfdk_aac -strict -2 -ar 44100 -ac 1 \
					-f flv $RTMP_URL \
					>$LOG_FILE 2>&1 &
fi

while true;do
	sleep 2
	SELF_PID=$$
	FFMPEG_PID=`ps --ppid $SELF_PID | grep ffmpeg | awk '{if($1~/[0-9]+/) print $1}'`
	if [ "$FFMPEG_PID" == "" ];then
		exit;
	fi
done