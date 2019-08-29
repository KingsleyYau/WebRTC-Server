#!/bin/sh
# RTP Stream to RTMP Stream script
# Author:	Max.Chiu

FFMPEG=/Users/max/Documents/tools/build/bin/ffmpeg

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

$FFMPEG -probesize 90000 -protocol_whitelist "file,http,https,rtp,rtcp,udp,tcp,tls" -thread_queue_size 1024 \
				-re -i test.mp4 \
				-vcodec libx264 -preset superfast -g 12 -profile:v baseline -level 3.0 -b:v 1000k -an -payload_type 102 -ssrc 0x12345678 -cname video -f rtp $RTP_URL:33333 \
				-acodec opus -strict -2 -ac 1 -vn -payload_type 111 -ssrc 0x12345679 -cname audio -f rtp $RTP_URL:33333 > 1.sdp