#!/bin/sh
# Install mediasever script
# Author: Max.Chiu
# Date: 2019/11/11

echo -e "############## Updating mediaserver ##############"
CUR_DIR=$(dirname $(readlink -f "$0"))/..
cd $CUR_DIR

DEST_PATH="/app/live/mediaserver"

$DEST_PATH/script/stop_test_service.sh
$DEST_PATH/script/stop_all_service.sh

# Copy File
echo "# Update Files......"
cp -rf * $DEST_PATH || exit 1

echo -e "############## Updating mediaserver [\033[32mOK\033[0m] ##############"

$DEST_PATH/script/restart_all_service.sh
$DEST_PATH/script/restart_test_service.sh