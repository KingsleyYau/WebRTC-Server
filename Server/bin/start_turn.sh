#!/bin/sh
# Start turnserver shell
# Author: Max.Chiu
# 2018/12/25
#

# Start redis
APP_DIR=/root/Max/webrtc/thirdparty/coturn-4.5.1.1
APP=$APP_DIR/bin/turnserver
cd $APP_DIR
$APP -a -v -b db/turndb -c etc/turnserver.conf -L 192.168.88.133
cd -