#!/bin/sh
# Start mediaserver shell
# Author: Max.Chiu
# 2019/08/09
#

APP_DIR=/root/Max/mediaserver
SCRIPT_DIR=$APP_DIR/script
cd $SCRIPT_DIR

# Start turnserver
echo "# Turnserver starting......"
#nohup ./start_turnserver.sh >/dev/null 2>&1 &
echo "# Turnserver start OK"

sleep 3

# Start mediaserver
echo "# Mediaserver starting......"
nohup ./start_mediaserver.sh >/dev/null 2>&1 &
PID=$!
echo "# Mediaserver start OK, pid: $PID"

cd -