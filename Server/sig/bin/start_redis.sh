#!/bin/sh
# Start app shell
# Author: Max.Chiu
# 2018/12/25
#

# Start redis
REDIS_DIR=/Users/max/Documents/Project/redis
REDIS=$REDIS_DIR/bin/redis-server
cd $REDIS_DIR
$REDIS ./conf/redis.conf
cd -