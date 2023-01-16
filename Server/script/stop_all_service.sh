#!/bin/sh
# Start mediaserver shell
# Author: Max.Chiu
# Date: 2019/12/11

APP_DIR=$(dirname $(readlink -f "$0"))/..
cd $APP_DIR

echo "# Deamon stopping......"
PID=`cat $APP_DIR/run/deamon.pid 2>/dev/null`
if [ ! $PID == "" ];then
  kill -9 $PID
  rm -f $APP_DIR/run/deamon.pid
fi
echo "# Deamon stop OK"

echo "# Mediaserver stopping......"
./script/stop_mediaserver.sh
echo "# Mediaserver stop OK"

echo "# Turnserver stopping......"
./script/stop_turnserver.sh
echo "# Turnserver stop OK"

# Stop all script
killall -9 ffmpeg >/dev/null 2>&1

cd - >/dev/null 2>&1