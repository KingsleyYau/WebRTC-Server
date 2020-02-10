#!/bin/sh
# WebRTC MediaServer compile whole script
# Author: Max.Chiu

BUILD_PATH=$(pwd)/build

function Usage {
  echo "Usage : ./compile.sh [IS_CLEAN], example: ./compile.sh noclean"
}

NOCLEAN="$1"
if [ "$NOCLEAN" == "noclean" ]; then
  echo "# Build WebRTC MediaServer without clean"
else
  echo "# Build WebRTC MediaServer with clean"
  NOCLEAN=""
fi

# Build dependence
cd dep
# Build coturn
./build-coturn.sh $BUILD_PATH $NOCLEAN || exit 1
# Build ffmpeg
./build-ffmpeg.sh $BUILD_PATH $NOCLEAN || exit 1
cd -

# Build mediaserver
cd MediaServer
./compile.sh $BUILD_PATH $NOCLEAN || exit 1
cd -