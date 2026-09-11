#!/bin/sh
# Install mediasever script
# Author: Max.Chiu
# Date: 2019/11/11

echo -e "############## Updating mediaserver ##############"
DEBUG=
if [ ! "$1" == "" ]
then
  DEBUG=$1
fi
echo "# Updating(DEBUG=$DEBUG)"

USERNAME=mediaserver
CUR_DIR=$(dirname $(readlink -f "$0"))
cd $CUR_DIR

DEST_PATH="/app/live/mediaserver"
$DEST_PATH/script/stop_all_service.sh

sleep 20

echo -e "############## Updating camserver ##############"
# Copy File
echo "# Update Files......"
cp -rf file/* $DEST_PATH || exit 1
echo -e "############## Updating mediaserver [\033[32mOK\033[0m] ##############"

if [ "$DEBUG" == "debug" ];then
  rm -rf $DEST_PATH/log/mediaserver/*
  echo -e "############## [Debug] Remove mediaserver log [\033[32mOK\033[0m] ##############"
fi

USER=`whoami`
if [ "$USER" == "root" ];then
  #sudo yum install -y gdb
  chown -R mediaserver:mediaserver $DEST_PATH || exit 1
  su mediaserver -c $DEST_PATH/script/start_all_service.sh
else
  $DEST_PATH/script/start_all_service.sh
fi