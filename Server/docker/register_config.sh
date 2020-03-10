#!/bin/sh
# Docker run registry script
# Author: Max.Chiu
# Date: 2020/03/04

CUR_DIR=$(dirname $(readlink -f "$0"))
cp -f ~/.bash_profile ~/.bash_profile.bak

FILE=~/.bash_profile
touch $FILE

REGISTRY_DATA_DIR=$(docker container inspect registry | jq -r '.[] | .Mounts | .[].Source ')
REGISTRY_DATA_DIR=$REGISTRY_DATA_DIR/docker/registry/v2/

LINE=$(sed -n -e '/REGISTRY_DATA_DIR=/p' $FILE)
if [ "$LINE" == "" ];then
cat >> $FILE <<EOF
# Registry
export REGISTRY_DATA_DIR=$REGISTRY_DATA_DIR
EOF
else
  sed -e 's/REGISTRY_DATA_DIR=(.*)/REGISTRY_DATA_DIR=$REGISTRY_DATA_DIR/g' $FILE
fi