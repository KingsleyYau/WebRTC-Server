#!/bin/sh
# Docker run registry script
# Author: Max.Chiu
# Date: 2020/03/04

CUR_DIR=$(dirname $(readlink -f "$0"))

result=$(docker ps -a | grep mediaserver)
if [ "$result" != "" ];then
  echo $result
fi

docker container run -d --restart always \
  -p 5000:5000 \
  --name registry \
  -e REGISTRY_STORAGE_DELETE_ENABLED=true \
  registry