#!/bin/sh
# Stop mediaserver shell
# Author: Max.Chiu
# Date: 2019/12/11

APP_DIR=$(dirname $(readlink -f "$0"))/..
cd $APP_DIR

echo "# Mediaserver stopping......"
./script/stop_mediaserver.sh
echo "# Mediaserver stop OK"

echo "# Mediaserver(Camshare) stopping......"
./script/stop_mediaserver_camshare.sh
echo "# Mediaserver(Camshare) stop OK"

echo "# Turnserver stopping......"
./script/stop_turnserver.sh
echo "# Turnserver stop OK"

# Stop all script
killall -9 ffmpeg >/dev/null 2>&1

cd - >/dev/null 2>&1