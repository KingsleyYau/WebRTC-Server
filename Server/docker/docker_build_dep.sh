#!/bin/sh
# Docker mediasever_dep script
# Author: Max.Chiu
# Date: 2020/03/04

CUR_DIR=$(dirname $(readlink -f "$0"))

function build_dep {
  docker image build -t mediaserver_dep -f DepDockerfile .
}
build_dep