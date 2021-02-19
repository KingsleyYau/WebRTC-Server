#!/bin/sh
# Stop mediaserver(Test) shell
# Author: Max.Chiu
# Date: 2019/08/09

APP_DIR=$(dirname $(readlink -f "$0"))/..
cd $APP_DIR
APP_PID=`cat $APP_DIR/run/mediaserver_test.pid 2>/dev/null`
EXIT=0

CheckAndWait() {
  for (( i=1; i<5; i++))
  do  
    wait=0
    PID=`ps -ef | grep $APP_PID | grep -v grep`
    if [ -n "$PID" ]; then
      wait=1
    fi
    
    if [ "$wait" == 1 ]; then
      # 
      echo "# Waiting Mediaserver(Test) to exit...... ($APP_PID)"
      sleep 1
    else
    echo "# Mediaserver(Test) already exit...... ($APP_PID)"
      EXIT=1
      break
    fi 
  done
  return $exit
}

# Stop mediaserver
if [ -n "$(echo $APP_PID| sed -n "/^[0-9]\+$/p")" ];then
  echo "# Stop Mediaserver(Test) ($APP_PID) "
  kill $APP_PID
  CheckAndWait
  if [ $EXIT == 0 ];then
    echo "# Stop Mediaserver(Test) force ($APP_PID) "
    kill -9 $APP_PID
  fi
  rm -f $APP_DIR/run/mediaserver_test.pid
fi

cd - >/dev/null 2>&1