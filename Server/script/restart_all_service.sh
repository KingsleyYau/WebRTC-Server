#!/bin/sh
# Start mediaserver shell
# Author: Max.Chiu
# Date: 2019/12/11

APP_DIR=$(dirname $(readlink -f "$0"))/..
cd $APP_DIR

./script/stop_all_service.sh

sleep 1

echo "# Turnserver starting......"
nohup ./script/start_turnserver.sh >/dev/null 2>&1 &
echo "# Turnserver start OK"

sleep 1

echo "# Mediaserver starting......"
nohup ./script/start_mediaserver.sh >/dev/null 2>&1 &
sleep 1
PID=`cat ./run/mediaserver.pid`
echo "# Mediaserver start OK, shell: $!, pid: $PID"

cd - >/dev/null 2>&1