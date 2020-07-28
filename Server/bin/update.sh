#!/bin/sh
# Install mediasever script
# Author: Max.Chiu
# Date: 2019/11/11

echo -e "############## Updating mediaserver ##############"
DEMO=0
if [ ! "$1" == "" ]
then
  DEMO=$1
fi

USERNAME=mediaserver
CUR_DIR=$(dirname $(readlink -f "$0"))
cd $CUR_DIR

DEST_PATH="/app/live/mediaserver"

$DEST_PATH/script/stop_test_service.sh
$DEST_PATH/script/stop_all_service.sh

# Copy File
echo "# Update Files......"
cp -rf file/* $DEST_PATH || exit 1

# Change Own
if [ "$DEMO" == 0 ]
then
  chown -R mediaserver:mediaserver $DEST_PATH
  chown -R mediaserver:mediaserver /tmp/webrtc
fi

echo -e "############## Updating mediaserver [\033[32mOK\033[0m] ##############"

if [ "$DEMO" == 0 ]
then
  sudo -u mediaserver $DEST_PATH/script/restart_all_service.sh
else
  $DEST_PATH/script/restart_all_service.sh
fi


#$DEST_PATH/script/restart_test_service.sh