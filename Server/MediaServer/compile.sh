#!/bin/sh
# MediaServer compile whole script
# Author:	Max.Chiu

BUILD_PATH="$1"
if [ "$BUILD_PATH" == "" ]; then
  echo "# BUILD_PATH must be set, like: /root/mediaserver/build"
  exit 1
fi

NOCLEAN="$2"
DEBUG="$3"
if [ "$NOCLEAN" == "noclean" ]; then
  echo "# Compile MediaServer without clean"
  DEBUG=
else
  echo "# Compile MediaServer with clean($DEBUG)"
  NOCLEAN="clean"
fi

if [ "$NOCLEAN" != "noclean" ]; then
	chmod +x ./configure || exit 1
	./configure $BUILD_PATH $DEBUG || exit 1
  make clean
fi

NEW_BUILD_PATH=`echo $BUILD_PATH | sed 's#\/#\\\/#g'`
sed -i 's/\(export BUILD_PATH = \).*/\1'"$NEW_BUILD_PATH"'/g' Makefile
make || exit 1

mkdir -p $BUILD_PATH/bin
cp mediaserver $BUILD_PATH/bin || exit 1
cp tools/wscat $BUILD_PATH/bin || exit 1
mkdir -p $BUILD_PATH/etc
cp etc/* $BUILD_PATH/etc || exit 1
