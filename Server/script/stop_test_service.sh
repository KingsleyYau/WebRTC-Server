#!/bin/sh
# Start mediaserver shell
# Author: Max.Chiu
# Date: 2019/12/11

APP_DIR=$(dirname $(readlink -f "$0"))/..
cd $APP_DIR

echo "# Mediaserver stoping......"
./script/stop_mediaserver.sh
echo "# Mediaserver stop OK"

echo "# Mediaserver(Camshare) stoping......"
./script/stop_mediaserver_camshare.sh
echo "# Mediaserver(Camshare) stop OK"

cd - >/dev/null 2>&1