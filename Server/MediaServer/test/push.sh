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
#set -x
mkdir -p tmp
FILE_LIST=()
FILE_MD5_SUM=`echo $RTP_URL | md5sum | awk '{print $1}'`
FILE_LIST_TMP="tmp/$FILE_MD5_SUM"
find c -type f > $FILE_LIST_TMP 
while read LINE; do
  #echo "# FILE_COUNT:${#FILE_LIST[@]}"
  FILE_LIST[${#FILE_LIST[@]}]="$LINE"
done < $FILE_LIST_TMP
FILE_COUNT=${#FILE_LIST[@]}
#echo "# FILE_COUNT:$FILE_COUNT"
NUM=$FILE_COUNT
INDEX=$(($(cat /dev/urandom 2>/dev/null | head -n 10 | cksum | awk -F ' ' '{print $1}')%${NUM}))
#echo "# $RTP_URL $INDEX"
FILE=${FILE_LIST[${INDEX}]}
TRANSCODE=0
LOGFILE=log/$$.log
PID=$$
echo "# $PID $RTP_URL $INDEX $LOGFILE $FILE" 
if [ "$TRANSCODE" -eq "1" ]
then
$FFMPEG -probesize 90000 -protocol_whitelist "file,http,https,rtp,rtcp,udp,tcp,tls" -thread_queue_size 1024 \
				-re -stream_loop -1 -i $FILE \
				-vcodec libx264 -preset superfast -profile:v baseline -level 3.0 -b:v 300k -bsf:v h264_mp4toannexb -an -payload_type 102 -ssrc 0x12345678 -cname video -force_key_frames 'expr:gte(t,n_forced*5)' -f rtp $RTP_URL \
				-acodec opus -strict -2 -ac 1 -vn -payload_type 111 -ssrc 0x12345679 -cname audio -f rtp $RTP_URL >/dev/null 2>&1 &
else
$FFMPEG -probesize 90000 -protocol_whitelist "file,http,https,rtp,rtcp,udp,tcp,tls" -thread_queue_size 1024 \
				-re -stream_loop -1 -i $FILE \
				-vcodec copy -bsf:v h264_mp4toannexb -an -payload_type 102 -ssrc 0x12345678 -cname video -f rtp $RTP_URL \
				-acodec opus -strict -2 -ac 1 -vn -payload_type 111 -ssrc 0x12345679 -cname audio -f rtp $RTP_URL >${LOGFILE} 2>&1 &
fi

wait $!
#while true;do
#	sleep 2
#	SELF_PID=$$
#	FFMPEG_PID=`ps --ppid $SELF_PID | grep ffmpeg | awk '{if($1~/[0-9]+/) print $1}'`
#	if [ "$FFMPEG_PID" == "" ];then
#		exit;
#	fi
#done