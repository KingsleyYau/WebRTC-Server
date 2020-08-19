#!/bin/sh
# MediaServer compile whole script
# Author:	Max.Chiu

echo "# Install dependent tools ..."
sudo yum install -y automake libtool gcc-c++.x86_64 gtk-doc.x86_64 glib2-devel.x86_64 boost-devel.x86_64 libidn-devel.x86_64 libselinux-devel.x86_64 libuuid-devel.x86_64

wget http://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
sudo rpm -ivh epel-release-latest-7.noarch.rpm
sudo yum install -y jq cmake3

BUILD_PATH="$1"
if [ "$BUILD_PATH" == "" ]; then
  echo "# BUILD_PATH must be set, like: /root/mediaserver/build"
fi

NOCLEAN="$2"
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

NEW_BUILD_PATH=`echo $BUILD_PATH | sed 's#\/#\\\/#g'`
sed -i 's/\(export BUILD_PATH = \).*/\1'"$NEW_BUILD_PATH"'/g' Makefile
make || exit 1

mkdir -p $BUILD_PATH/bin
cp mediaserver $BUILD_PATH/bin || exit 1
mkdir -p $BUILD_PATH/etc
cp etc/* $BUILD_PATH/etc || exit 1