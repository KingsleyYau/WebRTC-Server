#!/bin/sh
# RTP Stream to RTMP Stream script
# Author:	Max.Chiu

FFMPEG=/root/Max/webrtc/ffmpeg/build/bin/ffmpeg

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

trap 'Clean; exit' SIGTERM
function Clean() {
	SELF_PID=$$
	FFMPEG_PID=`ps --ppid $SELF_PID | grep ffmpeg | awk '{if($1~/[0-9]+/) print $1}'`
	if [ ! "$FFMPEG_PID" == "" ];then
		kill -9 $FFMPEG_PID
	fi
}

$FFMPEG -protocol_whitelist "file,http,https,rtp,rtcp,udp,tcp,tls" -thread_queue_size 1024 -re -i $SDP_FILE -vcodec libx264 -preset superfast -profile:v baseline -level 3.0 -c:a libfdk_aac -strict -2 -ar 44100 -ac 1 -f flv $RTMP_URL >$SDP_FILE.log 2>&1 &

while true;do
	sleep 2
	SELF_PID=$$
	FFMPEG_PID=`ps --ppid $SELF_PID | grep ffmpeg | awk '{if($1~/[0-9]+/) print $1}'`
	if [ "$FFMPEG_PID" == "" ];then
		exit;
	fi
done