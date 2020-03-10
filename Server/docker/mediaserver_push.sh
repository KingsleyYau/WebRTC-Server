#!/bin/sh
# Docker mediasever push script
# Author: Max.Chiu
# Date: 2020/03/04

CUR_DIR=$(dirname $(readlink -f "$0"))
SRC_DIR=$CUR_DIR/..

VERSION=`cat $SRC_DIR/version.json | jq -c '.version' `
VERSION=`echo $VERSION | sed s/\"//g`
echo -e "VERSION:[\033[32m$VERSION\033[0m]"

HOST=192.168.88.133:5000

VERSION_TAG=$HOST/mediaserver:$VERSION
docker tag mediaserver $VERSION_TAG
docker push $VERSION_TAG

LATEST_TAG=$HOST/mediaserver:latest
docker tag mediaserver $LATEST_TAG
docker push $LATEST_TAG

result=$(docker images | grep "<none>" | awk -F ' ' '{print $3}')
if [ "$result" != "" ];then
  echo -e "# Remove old images [\033[33m$result\033[0m]"
  docker rmi $result --force
fi