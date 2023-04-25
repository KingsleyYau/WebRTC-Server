#!/bin/sh
# Start mediaserver shell
# Author: Max.Chiu
# Date: 2019/12/11

APP_DIR=$(dirname $(readlink -f "$0"))/..
cd $APP_DIR

./script/stop_all_service.sh

sleep 10

echo "# Turnserver starting......"
nohup ./script/start_turnserver.sh >/dev/null 2>&1 &
sleep 3
PID=`cat ./run/turnserver.pid 2>/dev/null`
echo "# Turnserver start finish, shell: $!, pid: $PID"
if [ ! $PID == "" ]
then
  echo -e "############## Restart turnserver [\033[32mOK\033[0m] ##############"
else
  echo -e "############## Restart turnserver [\033[31mFail\033[0m] ##############"
fi


echo "# Mediaserver starting......"
nohup ./script/start_mediaserver.sh >/dev/null 2>&1 &
sleep 10
PID=`cat ./run/mediaserver.pid 2>/dev/null`
echo "# Mediaserver start finish, shell: $!, pid: $PID"
if [ ! $PID == "" ]
then
  echo -e "############## Restart mediaserver [\033[32mOK\033[0m] ##############"
else
  echo -e "############## Restart mediaserver [\033[31mFail\033[0m] ##############"
fi

echo "# Mediaserver(Camshare) starting......"
nohup ./script/start_mediaserver_camshare.sh >/dev/null 2>&1 &
sleep 10
PID=`cat ./run/mediaserver_camshare.pid 2>/dev/null`
echo "# Mediaserver(Camshare) start finish, shell: $!, pid: $PID"
if [ ! $PID == "" ]
then
  echo -e "############## Restart mediaserver(Camshare) [\033[32mOK\033[0m] ##############"
else
  echo -e "############## Restart mediaserver(Camshare) [\033[31mFail\033[0m] ##############"
fi


echo "# Deamon starting......"
nohup watch -n 300 /app/live/mediaserver/script/deamon.sh >/dev/null 2>&1 &
echo $! > ./run/deamon.pid
sleep 3
echo "# Deamon start finish, shell: $!"
if [ ! $! == "" ]
then
  echo -e "############## Restart deamon [\033[32mOK\033[0m] ##############"
else
  echo -e "############## Restart deamon [\033[31mFail\033[0m] ##############"
fi

cd - >/dev/null 2>&1

exit 0