#!/bin/sh
# Start mediaserver test shell
# Author: Max.Chiu
# Date: 2019/12/11

APP_DIR=$(dirname $(readlink -f "$0"))/..
cd $APP_DIR

echo "# Mediaserver(Test) stoping......"
./script/stop_mediaserver_test.sh
echo "# Mediaserver(Test) stop OK"
sleep 1

echo "# Mediaserver(Test) starting......"
rm log/mediaserver_test -rf 
nohup ./script/start_mediaserver_test.sh >/dev/null 2>&1 &
sleep 3
PID=`cat ./run/mediaserver_test.pid 2>/dev/null `
echo "# Mediaserver(Test) start finish, shell: $!, pid: $PID"
if [ ! $PID == "" ]
then
  echo -e "############## Restart mediaserver(Test) [\033[32mOK\033[0m] ##############"
else
  echo -e "############## Restart mediaserver(Test) [\033[31mFail\033[0m] ##############"
fi

cd - >/dev/null 2>&1

exit 0