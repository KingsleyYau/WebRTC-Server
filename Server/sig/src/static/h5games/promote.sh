#!/bin/sh
# Send promotion mail 
# Author:	Max.Chiu

function Usage {
  echo "Usage : ./promote.sh [Mail Address]"
  echo "Example : ./promote.sh ABC@gmail.com"
}

EMAIL=""
if [ ! "$1" == "" ]
then
	EMAIL=$1
else
	Usage;
	exit 1;
fi


NAME=`echo $EMAIL | awk -F '@' '{print $1}'` 
cp -f promote.html.example promote.html
sed -i "s/\[SEND_NAME\]/$NAME/g" promote.html
SUBJECT=$(echo -e "Hi, $NAME. We found a lot of interesting games for you.")
mutt -e 'set from=Maxzoon<max@maxzoon.cn>' -e 'set content_type=text/html' -s "$SUBJECT" $EMAIL < promote.html