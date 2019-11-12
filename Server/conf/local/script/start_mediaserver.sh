#!/bin/sh
# Start mediaserver shell
# Author: Max.Chiu
# 2019/08/09
#

# Start mediaserver
APP_DIR=/root/Max/mediaserver
APP_EXE=$APP_DIR/bin/mediaserver
APP_CONFIG=$APP_DIR/etc/mediaserver.config
cd $APP_DIR
$APP_EXE -f $APP_CONFIG
cd -