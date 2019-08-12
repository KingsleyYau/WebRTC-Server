#!/bin/sh

echo "# Install dependent tools ..."
sudo yum install -y automake libtool gcc-c++.x86_64 gtk-doc.x86_64 glib2-devel.x86_64 boost-devel.x86_64 libidn-devel.x86_64 libselinux-devel.x86_64

BUILD_PATH=$(pwd)/../build

NOCLEAN="$1"
if [ "$NOCLEAN" == "noclean" ]; then
	echo "# Build MediaServer without clean"
else
  echo "# Build MediaServer with clean"
  NOCLEAN=""
fi

if [ "$NOCLEAN" != "noclean" ]; then
	chmod +x ./configure || exit 1
	./configure $BUILD_PATH || exit 1
  make clean
fi
make || exit 1

cp media-server $BUILD_PATH/bin || exit 1
cp media-server.config $BUILD_PATH/etc || exit 1