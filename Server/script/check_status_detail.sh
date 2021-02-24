#!/bin/sh
# Watch mediaserver script
# Author: Max.Chiu
# Date: 2019/08/13

watch -n 1 -c "ps -efww | grep -v tail | grep -v grep | grep 'mediaserver' | grep -v 'sshd\|js\|ffmpeg\|start_\|check_status\|deamon' | sort -k 8;\
"