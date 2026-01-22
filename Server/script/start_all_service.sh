#!/bin/sh
# mediaserver start script
# Author: Max.Chiu
# Date: 2019/12/11

SCRIPT_DIR=$(dirname $(readlink -f "$0"))
APP_DIR=$SCRIPT_DIR/..
cd $APP_DIR

echo "# Turnserver starting......"
nohup ./script/start_turnserver.sh >/dev/null 2>&1 &
sleep 3
PID=`cat ./run/turnserver.pid 2>/dev/null`
echo "# Turnserver start finish, shell: $!, pid: $PID"
if [ ! $PID == "" ]
then
  echo -e "############## Start turnserver [\033[32mOK\033[0m] ##############"
else
  echo -e "############## Start turnserver [\033[31mFail\033[0m] ##############"
fi

sleep 1

echo "# Mediaserver starting......"
nohup ./script/start_mediaserver.sh >/dev/null 2>&1 &
sleep 10
PID=`cat ./run/mediaserver.pid 2>/dev/null`
echo "# Mediaserver start finish, shell: $!, pid: $PID"
if [ ! $PID == "" ]
then
  echo -e "############## Start mediaserver [\033[32mOK\033[0m] ##############"
else
  echo -e "############## Start mediaserver [\033[31mFail\033[0m] ##############"
fi

echo "# Deamon starting......"
nohup watch -n 300 "$SCRIPT_DIR/deamon.sh" >/dev/null 2>&1 &
echo $! > ./run/deamon.pid
sleep 3
echo "# Deamon start finish, shell: $!"
if [ ! $! == "" ]
then
  echo -e "############## Start deamon [\033[32mOK\033[0m] ##############"
else
  echo -e "############## Start deamon [\033[31mFail\033[0m] ##############"
fi

cd - >/dev/null 2>&1

exit 0