#!/bin/sh
# Start Chrome with debug log script
# Author:	Max.Chiu

LOG_PATH="/Users/max/Library/Application\ Support/Google/Chrome"
LOG_FILE="chrome_debug"
rm -f $LOG_PATH/$LOG_FILE

nohup /Applications/Google\ Chrome.app/Contents/MacOS/Google\ Chrome --enable-logging --vmodule=*/webrtc/*=1 &
open $LOG_PATH