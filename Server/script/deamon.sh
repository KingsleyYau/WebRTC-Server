#!/bin/sh
# mediaserver Deamon Script
# Author:	Max.Chiu

APP_DIR=$(dirname $(readlink -f "$0"))/..
cd $APP_DIR


# define reboot log path
REBOOT_LOG_FILE_TIME=$(date +%Y-%m-%d)
REBOOT_LOG_DIR_PATH=./log/deamon
mkdir -p $REBOOT_LOG_DIR_PATH
REBOOT_LOG_FILE_PATH=${REBOOT_LOG_DIR_PATH}/mediaserver.$REBOOT_LOG_FILE_TIME.log


# print_log function
function print_log()
{
  print_body=$1
  if [ -n "$REBOOT_LOG_FILE_PATH" ]; then
    log_time=$(date "+[%Y-%m-%d %H:%M:%S]")
    echo "$log_time mediaserver_deamon.sh: $print_body" >> $REBOOT_LOG_FILE_PATH
  else
    echo "$log_time mediaserver_deamon.sh: $print_body"
  fi
}


function create_mail {
  NOW=$(date +"%Y-%m-%d %H:%M:%S")
  MAIL_FILE=/tmp/mediaserver_reboot_mail
  echo "Subject: Mediaserver Reboot [$NOW]" > $MAIL_FILE
  echo "from:Max<max.chu@qpidnetwork.com>" >> $MAIL_FILE
  echo "to:Max<max.chiu@qpidnetwork.com>" >> $MAIL_FILE
  tail -n 4 $REBOOT_LOG_FILE_PATH >> $MAIL_FILE
  sendmail -f max.chiu@qpidnetwork.com -t max.chiu@qpidnetwork.com < $MAIL_FILE
  rm -f $MAIL_FILE
}


IS_REBOOT=0
MEDIASERVER_PID=`cat ./run/mediaserver.pid 2>/dev/null`
TURNSERVER_PID=`cat ./run/turnserver.pid 2>/dev/null`

print_log "# START #################################################################################### "
export SERVER="127.0.0.1"

export TEST_REQ="{\"id\":0,\"route\":\"imRTC/sendPing\"}"
export TEST_RES="{\"data\":null,\"errmsg\":\"\",\"errno\":0,\"id\":0,\"route\":\"imRTC/sendPing\"}"
WS_RES=$(/app/live/mediaserver/bin/wscat -c ws://${SERVER}:9881 -t 10 -x "$TEST_REQ" 2>&1)
if [ "$WS_RES" == "$TEST_RES" ];then
  print_log "# CHECK_WS:OK"
else
  print_log "# CHECK_WS:FAIL WS_RES:$WS_RES"
  IS_REBOOT=1
fi  

TURN_CONNECT=$(bash -c 'cat < /dev/null > /dev/tcp/${SERVER}/3478' 2>&1 | grep -v connect)
if [ "$TURN_CONNECT" == "" ];then
  print_log "# CHECK_TURN:OK"
else
  print_log "# CHECK_TURN:FAIL TURN_CONNECT:$TURN_CONNECT"
  IS_REBOOT=1
fi


# reboot
#echo "is_reboot: $is_reboot"
if [ $(($IS_REBOOT)) -gt 0 ]; then
  REBOOT_TIME=$(date +%Y-%m-%d-%H-%M-%S)
  # print log
  print_log "# Reboot now... "
  
  # Send Mail
  create_mail
  
  # Dump thread
  if [ ! $"MEDIASERVER_PID" == "" ];then
    gdb -p $MEDIASERVER_PID -x ./script/dump_thread_bt.init
    mv /tmp/mediaserver.log.dump ${REBOOT_LOG_DIR_PATH}/mediaserver.${REBOOT_TIME}.dump
  fi

  # Restart server
  /bin/su -l mediaserver -c ./script/restart_all_service.sh
fi
print_log "# END #################################################################################### "


cd - >/dev/null 2>&1