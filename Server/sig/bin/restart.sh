#!/bin/sh
# Restart app shell
# Author: Max.Chiu
# 2018/12/25
#

# Restart app
APP_DIR=/Users/max/Documents/Project/node/app/bin
cd $APP_DIR
./stop.sh
./start.sh
cd -