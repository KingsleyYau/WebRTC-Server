#!/bin/sh
# Docker run mediasever_dep script
# Author: Max.Chiu
# Date: 2020/03/04

CUR_DIR=$(dirname $(readlink -f "$0"))

result=$(docker ps -a | grep mediaserver)
if [ "$result" != "" ];then
  echo $result
fi

#docker container run -d --restart always \
docker container run -d --rm \
  -p 9981:9981 \
  -p 3478:3478 \
  -p 3478:3478/udp \
  --name mediaserver \
  -v "$CUR_DIR/log":/app/live/mediaserver/log \
  mediaserver:latest