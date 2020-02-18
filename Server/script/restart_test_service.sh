#!/bin/sh
# Start mediaserver test shell
# Author: Max.Chiu
# Date: 2019/12/11

APP_DIR=$(dirname $(readlink -f "$0"))/..
cd $APP_DIR

echo "# Mediaserver(Test) stoping......"
./script/stop_mediaserver_test.sh
echo "# Mediaserver(Test) stop OK"

sleep 1

echo "# Mediaserver(Test) starting......"
rm log/mediaserver_test -rf 
nohup ./script/start_mediaserver_test.sh >/dev/null 2>&1 &
sleep 1
PID=`cat ./run/mediaserver_test.pid`
echo "# Mediaserver(Test) start OK, shell: $!, pid: $PID"

cd - >/dev/null 2>&1