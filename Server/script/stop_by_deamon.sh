#!/bin/sh
# Start mediaserver shell
# Author: Max.Chiu
# Date: 2019/12/11

APP_DIR=$(dirname $(readlink -f "$0"))/..
cd $APP_DIR

echo "# Mediaserver stoping......"
./script/stop_mediaserver.sh
echo "# Mediaserver stop OK"

echo "# Turnserver stoping......"
./script/stop_turnserver.sh
echo "# Turnserver stop OK"

# Stop all script
killall -9 ffmpeg >/dev/null 2>&1

cd - >/dev/null 2>&1