#!/bin/sh
# Stop turnserver shell
# Author: Max.Chiu
# Date: 2019/08/13

# Stop coturn
APP_DIR=$(dirname $(readlink -f "$0"))/..
cd $APP_DIR
APP_PID=`cat $APP_DIR/run/turnserver.pid 2>/dev/null`
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
      echo "# Waiting Turnserver to exit...... ($APP_PID)"
      sleep 1
    else
    echo "# Turnserver already exit...... ($APP_PID)"
      EXIT=1
      break
    fi 
  done
  return $exit
}

# Stop turnserver
if [ -n "$(echo $APP_PID| sed -n "/^[0-9]\+$/p")" ];then
  echo "# Stop Turnserver ($APP_PID) "
  kill $APP_PID
  CheckAndWait
  if [ $EXIT == 0 ];then
    echo "# Stop Turnserver force ($APP_PID) "
    kill -9 $APP_PID
  fi
  rm -f $APP_DIR/run/turnserver.pid
fi

cd - >/dev/null 2>&1