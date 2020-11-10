# !/bin/sh
for((;;)); do
CONECTIONS=$(netstat -antlop | grep 5555);
SYNC_RECV=$(netstat -antlop | grep 5555 | grep SYNC_RECV | wc -l);
TIME_WAIT=$(netstat -antlop | grep 5555 | grep TIME_WAIT | wc -l);
ESTABLISHED=$(netstat -antlop | grep 5555 | grep ESTABLISHED | wc -l);
printf "\r"
printf "ESTABLISHED:%5d, SYNC_RECV:%5d, TIME_WAIT:%5d" $ESTABLISHED $SYNC_RECV $TIME_WAIT
sleep 3;
done;
