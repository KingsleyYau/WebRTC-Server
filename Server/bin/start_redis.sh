#!/bin/sh
# Start redis shell
# Author: Max.Chiu
# 2018/12/25
#

# Start redis
APP_DIR=/root/Max/webrtc/thirdparty/redis
APP=$APP_DIR/bin/redis-server
cd $APP_DIR
$APP ./conf/redis.conf
cd -