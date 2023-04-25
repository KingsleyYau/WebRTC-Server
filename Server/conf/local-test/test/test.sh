#!/bin/sh
# RTMP stream to RTP stream script
# Author: Max.Chiu
# Date: 2020/1/07

APP_DIR=/app/live/mediaserver
FFMPEG=$APP_DIR/bin/ffmpeg

function Usage {
	echo "Usage : ./rtmp2rtp.sh [RTMP_URL] [RTP_URL] [VIDEO_PAYLOAD] [AUDIO_PAYLOAD] [TRANSCODE]"
	echo "Usage : ./rtmp2rtp.sh rtmp://127.0.0.1:1935/live/max rtp://127.0.0.1:12345 102 111 1"
}

RTMP_URL="rtmp://127.0.0.1:4000/cdn_standard/max0"
if [ ! "$1" == "" ]
then
	RTMP_URL=$1
fi

RTP_URL="rtp://127.0.0.1:12345"
if [ ! "$2" == "" ]
then
	RTP_URL=$2
fi

VIDEO_PAYLOAD="102"
if [ ! "$3" == "" ]
then
	VIDEO_PAYLOAD=$3
fi

AUDIO_PAYLOAD="111"
if [ ! "$4" == "" ]
then
	AUDIO_PAYLOAD=$4
fi

TRANSCODE=1
if [ ! "$5" == "" ]
then
	TRANSCODE=$5
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
	$FFMPEG -probesize 90000 -protocol_whitelist "file,http,https,rtp,rtcp,rtmp,udp,tcp,tls" -thread_queue_size 1024 -i $RTMP_URL \
				-v trace \
				-acodec opus -vn -payload_type $AUDIO_PAYLOAD -ssrc 0x12345679 -cname audio -strict -2 -ac 1 -f rtp "$RTP_URL"
				#-vcodec libx264 -an -payload_type $VIDEO_PAYLOAD -ssrc 0x12345678 -cname video -preset superfast -profile:v baseline -level 3.0 -g 15 -f rtp "$RTP_URL" 
				#-acodec opus -vn -payload_type $AUDIO_PAYLOAD -ssrc 0x12345679 -cname audio -strict -2 -ac 1 -f rtp "$RTP_URL"
else
	$FFMPEG -probesize 90000 -protocol_whitelist "file,http,https,rtp,rtcp,rtmp,udp,tcp,tls" -thread_queue_size 1024 -i $RTMP_URL \
				-v trace \
				-vcodec copy -an -payload_type $VIDEO_PAYLOAD -ssrc 0x12345678 -cname video -f rtp "$RTP_URL" 
				-acodec opus -vn -payload_type $AUDIO_PAYLOAD -ssrc 0x12345679 -cname audio -strict -2 -ac 1 -f rtp "$RTP_URL" 
fi

while true;do
	sleep 2
	SELF_PID=$$
	FFMPEG_PID=`ps --ppid $SELF_PID | grep ffmpeg | awk '{if($1~/[0-9]+/) print $1}'`
	if [ "$FFMPEG_PID" == "" ];then
		exit;
	fi
done