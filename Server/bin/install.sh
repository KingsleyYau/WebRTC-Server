#!/bin/sh
# Install mediasever script
# Author: Max.Chiu
# Date: 2019/11/11

DEMO="$1"

echo "############## Start installing mediaserver ##############"
if [ $DEMO != "demo" ]; then
  sudo yum install -y boost-chrono.x86_64 boost-system.x86_64 boost-random.x86_64
fi

DEST_PATH="/app/live/mediaserver"
mkdir -p $DEST_PATH

# Copy Executable Files
echo "# Copy Executable Files......"
cp -rf bin $DEST_PATH
chmod -R 755 $DEST_PATH/bin/*

# Copy Config Files
echo "# Copy Config Files......"
cp -rf etc $DEST_PATH
chmod -R 744 $DEST_PATH/etc/*

# Copy Script Files
echo "# Copy Script Files......"
cp -rf script $DEST_PATH
chmod -R 755 $DEST_PATH/script/*

# Copy Var Files
echo "# Copy Var Files......"
cp -rf var $DEST_PATH
chmod -R 744 $DEST_PATH/var/*

# Create Run Dir
mkdir -p $DEST_PATH/run/

# Create Log Dir
mkdir -p $DEST_PATH/log/
mkdir -p $DEST_PATH/log/turnserver/

echo "############## Finish installing mediaserver ##############"