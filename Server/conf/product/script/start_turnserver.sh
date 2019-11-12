#!/bin/sh
# Start turnserver shell
# Author: Max.Chiu
# 2018/12/25
#

# Start coturn
APP_DIR=/app/live/mediaserver
APP_EXE=$APP_DIR/bin/turnserver
APP_CONFIG=$APP_DIR/etc/turnserver.conf
APP_DATABASE=$APP_DIR/var/db/turndb
cd $APP_DIR
$APP_EXE -a -v -b $APP_DATABASE -c $APP_CONFIG -L 0.0.0.0
cd -