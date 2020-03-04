#!/bin/sh
# Docker run mediasever_dep script
# Author: Max.Chiu
# Date: 2020/03/04

CUR_DIR=$(dirname $(readlink -f "$0"))

docker container run -d --rm -p 9877:9877 -p 9082:9082 mediaserver 