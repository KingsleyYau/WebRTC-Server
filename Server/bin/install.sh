#!/bin/sh
# Install mediasever script
# Author:	Max.Chiu

echo "############## Start installing mediaserver ##############"
DEST_PATH="/app/live/mediaserver"
mkdir -p $DEST_PATH

# Copy Executable Files
cp -rf bin $DEST_PATH
chmod -R 755 $DEST_PATH/bin/*

# Copy Config Files
cp -rf etc $DEST_PATH
chmod -R 744 $DEST_PATH/etc/*

# Copy Script Files
cp -rf script $DEST_PATH
chmod -R 755 $DEST_PATH/script/*

# Copy Var Files
cp -rf var $DEST_PATH
chmod -R 744 $DEST_PATH/var/*

echo "############## Finish installing mediaserver ##############"