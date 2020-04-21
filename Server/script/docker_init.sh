#!/bin/bash
# Docker init shell
# Author: Max.Chiu
# Date: 2019/08/13

trap 'stop_services; exit' SIGTERM SIGINT

start_services() {
  mkdir -p ./log/turnserver
  nohup ./script/start_turnserver.sh >/dev/null 2>&1 &
  nohup ./script/start_mediaserver_test.sh >/dev/null 2>&1 &
  nohup ./script/start_mediaserver.sh >/dev/null 2>&1 &
  nohup ./script/start_mediaserver_camshare.sh >/dev/null 2>&1 &
}

stop_services() {
  ./script/stop_mediaserver_camshare.sh
  ./script/stop_mediaserver.sh
  ./script/stop_mediaserver_test.sh
  ./script/stop_turnserver.sh
}

start_services

wait