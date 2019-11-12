#!/bin/sh
# Start mediaserver for camshare shell
# Author: Max.Chiu
# Date: 2019/08/09

# Start media-server
APP_DIR=/root/Max/mediaserver
APP_EXE=$APP_DIR/bin/mediaserver
APP_CONFIG=$APP_DIR/etc/mediaserver_camshare.config
cd $APP_DIR
$APP_EXE -f $APP_CONFIG
cd -