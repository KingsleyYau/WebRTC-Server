#!/bin/bash
# RTMP stream to RTP stream script
# Author: Max.Chiu
# Date: 2020/1/07

APP_DIR=$(dirname $(readlink -f "$0"))/..
FFMPEG=$APP_DIR/bin/ffmpeg

function Usage {
  echo "Usage : ./rtmp2rtp.sh [RTMP_URL] [RTP_URL] [VIDEO_PAYLOAD] [AUDIO_PAYLOAD] [TRANSCODE]"
  echo "Usage : ./rtmp2rtp.sh rtmp://127.0.0.1:1935/live/max rtp://127.0.0.1:12345 102 111 1"
}

RTMP_URL=""
if [ ! "$1" == "" ]
then
  RTMP_URL=$1
else
  Usage;
  exit 1;
fi

RTP_URL=""
if [ ! "$2" == "" ]
then
  RTP_URL=$2
else
  Usage;
  exit 1;
fi

VIDEO_PAYLOAD=""
if [ ! "$3" == "" ]
then
  VIDEO_PAYLOAD=$3
fi

AUDIO_PAYLOAD=""
if [ ! "$4" == "" ]
then
  AUDIO_PAYLOAD=$4
fi

TRANSCODE=1
if [ ! "$5" == "" ]
then
  TRANSCODE=$5
fi

function Clean() {
  SELF_PID=$$
  FFMPEG_PID=`ps --ppid $SELF_PID | grep ffmpeg | awk '{if($1~/[0-9]+/) print $1}'`
  if [ ! "$FFMPEG_PID" == "" ];then
    echo "# rtp2rtmp.sh kill -9 $FFMPEG_PID "
    kill -9 $FFMPEG_PID
  fi
  echo "# rmtp2rtp.sh $SELF_PID exit "
}
trap 'Clean; exit' SIGTERM

RTMP_STREAM=`echo $RTMP_URL | sed 's/rtmp:\/\/.*:[0-9]*\/\(.*\)/\1/g' | sed 's/\//_/g'`
RTP_PORT=`echo $RTP_URL | sed 's/rtp:\/\/.*:\([0-9]*\)/\1/g' | sed 's/\///g'`
LOG_FILE=/tmp/webrtc/rtmp2rtp_${RTMP_STREAM}_${RTP_PORT}.log


CPU_NUM=$(cat /proc/cpuinfo | grep processor | wc -l)
CPU=$(($(cat /dev/urandom 2>/dev/null | head -n 10 | cksum | awk -F ' ' '{print $1}')%${CPU_NUM}))
#if [ "1" -eq "1" ]
if [ "$TRANSCODE" -eq "1" ]
then
  taskset -c $CPU $FFMPEG -probesize 90000 -protocol_whitelist "file,http,https,rtp,rtcp,rtmp,udp,tcp,tls" \
          -thread_queue_size 1024 \
          -i $RTMP_URL \
          -vcodec libx264 -an -payload_type $VIDEO_PAYLOAD -ssrc 0x12345678 -cname video -preset superfast -profile:v baseline -level 3.0 -f rtp "$RTP_URL" \
          -acodec opus -vn -payload_type $AUDIO_PAYLOAD -ssrc 0x12345679 -cname audio -strict -2 -ac 1 -f rtp "$RTP_URL" \
          > $LOG_FILE 2>&1 &
else
  taskset -c $CPU $FFMPEG -probesize 90000 -protocol_whitelist "file,http,https,rtp,rtcp,rtmp,udp,tcp,tls" \
          -thread_queue_size 1024 \
          -i $RTMP_URL \
          -vcodec copy -bsf:v h264_mp4toannexb -an -payload_type $VIDEO_PAYLOAD -ssrc 0x12345678 -cname video -f rtp "$RTP_URL" \
          -acodec opus -vn -payload_type $AUDIO_PAYLOAD -ssrc 0x12345679 -cname audio -strict -2 -ac 1 -f rtp "$RTP_URL" \
          > $LOG_FILE 2>&1 &
fi

wait $!