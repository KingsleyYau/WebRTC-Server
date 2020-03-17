#!/bin/sh
# Start turnserver shell
# Author: Max.Chiu
# Date: 2019/08/13

# Start Turnserver
APP_DIR=$(dirname $(readlink -f "$0"))/..
APP_EXE=$APP_DIR/bin/turnserver
APP_CONFIG=$APP_DIR/etc/turnserver.conf

cd $APP_DIR
$APP_EXE -v -c $APP_CONFIG -L 0.0.0.0
cd -