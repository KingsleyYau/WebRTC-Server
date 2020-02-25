#!/bin/sh
# Start mediaserver shell
# Author: Max.Chiu
# Date: 2019/08/13

# Start mediaserver
APP_DIR=$(dirname $(readlink -f "$0"))/..
APP_EXE=$APP_DIR/bin/mediaserver
APP_CONFIG=$APP_DIR/etc/mediaserver_test.config
cd $APP_DIR
$APP_EXE -f $APP_CONFIG
cd -