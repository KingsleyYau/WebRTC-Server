#!/bin/sh
# mp3 metadata create script
# Author:	Max.Chiu

FFMPEG=/Users/max/Documents/tools/build/bin/ffmpeg

FILE_PATH=$1
FILE=`echo $FILE_PATH | awk -F '/' '{print $NF}'`

FILE_NAME=`echo $FILE | awk -F '.' '{print $1}' `
FILE_EX=`echo $FILE | awk -F '.' '{print $NF}' `

TITLE=`echo $FILE_NAME | awk -F '-' '{print $NF}' | awk -F '.' '{print $1}' `
ARTIST=`echo $FILE_NAME | awk -F '-' '{print $1}' `

OUTPUT=${FILE_NAME}_new.$FILE_EX
$FFMPEG -i $FILE_PATH -metadata title="$TITLE" -metadata artist="$ARTIST" -acodec copy -vcodec copy -y $OUTPUT
mv $OUTPUT $FILE_PATH