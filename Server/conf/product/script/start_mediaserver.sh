#!/bin/sh
# Start mediaserver shell
# Author: Max.Chiu
# Author: Max.Chiu
# Date: 2019/08/13

# Start media-server
APP_DIR=/home/samson/Max/mediaserver
APP_EXE=$APP_DIR/bin/mediaserver
APP_CONFIG=$APP_DIR/etc/mediaserver.config
cd $APP_DIR
$APP_EXE -f $APP_CONFIG
cd -