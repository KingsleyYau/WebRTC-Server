#!/bin/sh
# Start mediaserver shell
# Author: Max.Chiu
# Date: 2019/12/11

SCRIPT_DIR=$(dirname $(readlink -f "$0"))
APP_DIR=$SCRIPT_DIR/..
cd $APP_DIR

./script/stop_all_service.sh
sleep 10
./script/start_all_service.sh

cd - >/dev/null 2>&1

exit 0