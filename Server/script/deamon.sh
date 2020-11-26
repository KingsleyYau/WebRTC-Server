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
    echo -e "$log_time deamon.sh: $print_body" >> $REBOOT_LOG_FILE_PATH
  else
    echo -e "$log_time deamon.sh: $print_body"
  fi
}


function create_mail {
  NOW=$(date +"%Y-%m-%d %H:%M:%S")
  MAIL_FILE=/tmp/mediaserver_reboot_mail
  echo "Subject: Mediaserver Reboot [$NOW]" > $MAIL_FILE
  echo "from:Max<max.chu@qpidnetwork.com>" >> $MAIL_FILE
  echo "to:Max<269348928@qq.com>" >> $MAIL_FILE
  tail -n 40 $REBOOT_LOG_FILE_PATH >> $MAIL_FILE
  sendmail -f max.chiu@qpidnetwork.com -t 269348928@qq.com < $MAIL_FILE
  rm -f $MAIL_FILE
}


IS_REBOOT=0
MEDIASERVER_PID=`cat ./run/mediaserver.pid 2>/dev/null`
TURNSERVER_PID=`cat ./run/turnserver.pid 2>/dev/null`

print_log "#################################################################################### "
export SERVER="127.0.0.1"

# Check server port status
export TEST_REQ="{\"id\":0,\"route\":\"imRTC/sendPing\"}"
export TEST_RES="{\"data\":null,\"errmsg\":\"\",\"errno\":0,\"id\":0,\"route\":\"imRTC/sendPing\"}"

if [ $(($is_reboot)) == 0 ]; then
  i=0;
  timeout=10;
  for ((i=0; i<3; i++))
  do
    WS_RES=$(/app/live/mediaserver/bin/wscat -c ws://${SERVER}:9881 -t $timeout -x "$TEST_REQ" 2>&1)
    if [ "$WS_RES" == "$TEST_RES" ];then
      print_log "CHECK_WS:[\033[32mOK\033[0m]"
      break;
    else
      print_log "CHECK_WS:[\033[31mFAIL\033[0m] WS_RES:$WS_RES i:$i"
      timeout=$((timeout+10))
    fi  
  done

  if [ $(($i)) == 3 ];then
    IS_REBOOT=1
    print_log "# [\033[31mReboot because websocket timeout.\033[0m]"
  fi
fi

TURN_CONNECT=$(bash -c 'cat < /dev/null > /dev/tcp/${SERVER}/3478' 2>&1 | grep -v connect)
if [ "$TURN_CONNECT" == "" ];then
  print_log "CHECK_TURN:[\033[32mOK\033[0m]"
else
  print_log "CHECK_TURN:[\033[31mFAIL\033[0m] TURN_CONNECT:$TURN_CONNECT"
  IS_REBOOT=1
fi


# Show server status
print_log "-------------------- [\033[32mnetstat\033[0m] --------------------"
WSS_SOCKET=$(netstat -anpt 2>/dev/null | grep '9081\|9083\|9082' | grep 'ESTABLISHED' | wc -l )
WS_SOCKET=$(netstat -anpt 2>/dev/null | grep '9881\|9883\|9981' | grep 'ESTABLISHED' | wc -l )
print_log "WS:$WS_SOCKET WSS:$WSS_SOCKET " 
TCP_SOCKET=$(netstat -anpt 2>/dev/null | grep 'turnserver' | wc -l)
print_log "TCP Socket: $TCP_SOCKET" 
UDP_SOCKET=$(netstat -anpu 2>/dev/null | grep 'mediaserver\|turnserver' | wc -l)
print_log "UDP Socket: $UDP_SOCKET" 
FFMPEG_SOCKET=$(netstat -anptu 2>/dev/null | grep 'ffmpeg' | wc -l)
print_log "FFMPEG Socket: $FFMPEG_SOCKET" 
TIME_WAIT=$(netstat -antlpu 2>/dev/null | grep -c 'TIME_WAIT')
print_log "TIME_WAIT: $TIME_WAIT" 

TOP_HEAD=$(top -b -n 3 | grep -A 5 "load average" | tail -n 5)
SERVERS_STATUS=$(top -b -n 1 | grep 'turnserver\|mediaserver')
print_log "-------------------- [\033[32mtop\033[0m] --------------------\n$TOP_HEAD\n\n$SERVERS_STATUS"


# Reboot if needed
#IS_REBOOT=1
#echo "is_reboot: $is_reboot"
if [ $(($IS_REBOOT)) -gt 0 ]; then
  REBOOT_TIME=$(date +%Y-%m-%d-%H-%M-%S)
  # print log
  print_log "# [\033[31mReboot\033[0m] now... "
  
  # Send Mail
  #create_mail
  
  # Dump thread
  if [ ! $"MEDIASERVER_PID" == "" ];then
    print_log "# Dumping... "
    gdb -p $MEDIASERVER_PID -x ./script/dump_thread_bt.init
    mv /tmp/mediaserver.log.dump ${REBOOT_LOG_DIR_PATH}/mediaserver.${REBOOT_TIME}.dump
    print_log "# Dumping finish $APP_DIR/${REBOOT_LOG_DIR_PATH}/mediaserver.${REBOOT_TIME}.dump "
  fi

  # Restart server
  $APP_DIR/script/restart_by_deamon.sh >> $REBOOT_LOG_FILE_PATH
  
  print_log "# [\033[31mReboot\033[0m] finish"
fi

cd - >/dev/null 2>&1