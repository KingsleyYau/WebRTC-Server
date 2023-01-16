#!/bin/sh
# Start mediaserver shell
# Author: Max.Chiu
# Date: 2019/12/11

APP_DIR=$(dirname $(readlink -f "$0"))/..
cd $APP_DIR

echo "# Mediaserver(Test) stopping......"
./script/stop_mediaserver_test.sh
echo "# Mediaserver(Test) stop OK"

#echo "# Mediaserver(Camshare) stopping......"
#./script/stop_mediaserver_camshare.sh
#echo "# Mediaserver(Camshare) stop OK"

cd - >/dev/null 2>&1