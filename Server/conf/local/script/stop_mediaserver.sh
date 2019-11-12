#!/bin/sh
# Stop mediaserver shell
# Author: Max.Chiu
# 2019/08/09
#

APP_DIR=/root/Max/mediaserver
cd $APP_DIR
APP_PID=`cat $APP_DIR/var/mediaserver.pid`
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
      echo "# Waitting $APP_PID to exit......"
      sleep 1
    else
    echo "# $APP_PID already exit......"
      EXIT=1
      break
    fi 
  done
  return $exit
}

# Stop mediaserver
if [ -n "$(echo $APP_PID| sed -n "/^[0-9]\+$/p")" ];then
	echo "# kill $APP_PID "
	kill $APP_PID
	CheckAndWait
	if [ $EXIT == 0 ];then
		echo "# kill -9 $APP_PID"
		kill -9 $APP_PID
	fi
fi

cd -