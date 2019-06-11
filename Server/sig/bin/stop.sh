#!/bin/sh
# Stop app shell
# Author: Max.Chiu
# 2018/12/25
#

# Stop app
APP_DIR=/Users/max/Documents/Project/node/app
cd $APP_DIR
pm2 delete all && rm -rf log/* && rm -rf log-pm2/*
cd -