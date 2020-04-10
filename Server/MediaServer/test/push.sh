#!/bin/sh
# RTP Stream to RTMP Stream script
# Author:	Max.Chiu

FFMPEG=/app/live/mediaserver/bin/ffmpeg

function Usage {
	echo "Usage : ./push.sh rtp://127.0.0.1:10000"
}

RTP_URL=""
if [ ! "$1" == "" ]
then
	RTP_URL=$1
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

FILE=test.mp4
TRANSCODE=1
if [ "$TRANSCODE" -eq "1" ]
then
$FFMPEG -probesize 90000 -protocol_whitelist "file,http,https,rtp,rtcp,udp,tcp,tls" -thread_queue_size 1024 \
				-re -stream_loop -1 -i $FILE \
				-vcodec libx264 -preset superfast -r 12 -g 12 -profile:v baseline -level 3.0 -b:v 500k -an -payload_type 102 -ssrc 0x12345678 -cname video -force_key_frames 'expr:gte(t,n_forced*5)' -f rtp $RTP_URL \
				-acodec opus -strict -2 -ac 1 -vn -payload_type 111 -ssrc 0x12345679 -cname audio -f rtp $RTP_URL >/dev/null 2>&1 &
else
$FFMPEG -probesize 90000 -protocol_whitelist "file,http,https,rtp,rtcp,udp,tcp,tls" -thread_queue_size 1024 \
				-re -stream_loop -1 -i $FILE \
				-vcodec copy -an -payload_type 102 -ssrc 0x12345678 -cname video -f rtp $RTP_URL \
				-acodec opus -strict -2 -ac 1 -vn -payload_type 111 -ssrc 0x12345679 -cname audio -f rtp $RTP_URL >/dev/null 2>&1 &
fi

while true;do
	sleep 2
	SELF_PID=$$
	FFMPEG_PID=`ps --ppid $SELF_PID | grep ffmpeg | awk '{if($1~/[0-9]+/) print $1}'`
	if [ "$FFMPEG_PID" == "" ];then
		exit;
	fi
done