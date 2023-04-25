#!/bin/bash
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

function Clean() {
  SELF_PID=$$
  FFMPEG_PID=`ps --ppid $SELF_PID | grep ffmpeg | awk '{if($1~/[0-9]+/) print $1}'`
  if [ ! "$FFMPEG_PID" == "" ];then
    echo "# rtp2rtmp.sh kill -9 $FFMPEG_PID "
    kill -9 $FFMPEG_PID
  fi
  echo "# rtp2rtmp.sh $SELF_PID exit "
}
trap 'Clean; exit' SIGTERM SIGQUIT

RTMP_STREAM=`echo $RTMP_URL | sed 's/rtmp:\/\/.*:[0-9]*\/\(.*\)/\1/g' | sed 's/\//_/g'`

CPU_NUM=$(cat /proc/cpuinfo | grep processor | wc -l)
CPU=$(($(cat /dev/urandom 2>/dev/null | head -n 10 | cksum | awk -F ' ' '{print $1}')%${CPU_NUM}))
#if [ "1" -eq "1" ]
if [ "$TRANSCODE" -eq "1" ]
then
  taskset -c $CPU $FFMPEG -probesize 180000 -analyzeduration 10M -protocol_whitelist "file,http,https,rtp,rtcp,udp,tcp,tls" \
          -v error \
          -thread_queue_size 2048 \
          -reorder_queue_size 2048 \
          -max_delay 3000000 \
          -i $SDP_FILE \
          -max_muxing_queue_size 1024 \
          -map 0:v \
          -map 0:a \
          -vcodec libx264 -preset ultrafast -profile:v baseline -level 3.0 -g 12 \
          -acodec libfdk_aac -strict -2 -ar 44100 -ac 1 \
          -f flv $RTMP_URL \
          >$LOG_FILE 2>&1 &
else
  taskset -c $CPU $FFMPEG -probesize 180000 -analyzeduration 10M -protocol_whitelist "file,http,https,rtp,rtcp,udp,tcp,tls" \
          -v error \
          -thread_queue_size 2048 \
          -reorder_queue_size 2048 \
          -max_delay 3000000 \
          -i $SDP_FILE \
          -max_muxing_queue_size 1024 \
          -map 0:v \
          -map 0:a \
          -vcodec copy -bsf:v h264_mp4toannexb \
          -acodec libfdk_aac -strict -2 -ar 44100 -ac 1 \
          -f flv $RTMP_URL \
          >$LOG_FILE 2>&1 &
fi

wait $!