#!/bin/sh
# Stop mediaserver shell
# Author: Max.Chiu
# Date: 2019/08/13

APP_DIR=$(dirname $(readlink -f "$0"))/..
cd $APP_DIR
APP_PID=`cat $APP_DIR/run/mediaserver.pid 2>/dev/null`
PID=`ps -ef | grep "mediaserver -f" | grep "mediaserver.config" | awk '{print $2}'`
EXIT=0

CheckAndWait() {
  for (( i=1; i<5; i++))
  do  
    wait=0
    #PID=`ps -ef | grep $APP_PID | grep -v grep`
    PID=`ps -ef | grep "mediaserver -f" | grep "mediaserver.config" | awk '{print $2}'`
    if [ -n "$PID" ]; then
      wait=1
    fi
    
    if [ "$wait" == 1 ]; then
      echo "# Waiting Mediaserver to exit...... ($PID)"
      sleep 1
    else
      echo "# Mediaserver already exit......"
      EXIT=1
      break
    fi 
  done
  return $exit
}

# Stop mediaserver
if [ -n "$(echo $PID| sed -n "/^[0-9]\+$/p")" ];then
  echo "# Stop Mediaserver ($PID) "
  kill $PID
  CheckAndWait
  if [ $EXIT == 0 ];then
    echo "# Stop Mediaserver force ($PID) "
    kill -9 $PID
  fi
  rm -f $APP_DIR/run/mediaserver.pid
fi

cd - >/dev/null 2>&1