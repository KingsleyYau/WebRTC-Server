#!/bin/sh
# Start mediaserver shell
# Author: Max.Chiu
# Date: 2019/12/11

APP_DIR=/home/samson/Max/mediaserver
cd $APP_DIR

# Start turnserver
echo "# Turnserver starting......"
#nohup ./script/start_turnserver.sh >/dev/null 2>&1 &
echo "# Turnserver start OK"

sleep 3

# Stop mediaserver
echo "# Mediaserver stopping......"
./script/stop_mediaserver.sh
echo "# Mediaserver stop OK"

# Start mediaserver
echo "# Mediaserver starting......"
nohup ./script/start_mediaserver.sh >/dev/null 2>&1 &
PID=$!
echo "# Mediaserver start OK, pid: $PID"

cd -