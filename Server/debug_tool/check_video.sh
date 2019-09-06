#!/bin/sh
# RTP Stream to RTMP Stream script
# Author:	Max.Chiu

FFPROBE=/Users/max/Documents/tools/build/bin/ffprobe

FILE_PATH=$1

# Print video frame count
#$FFPROBE -v info -count_frames -select_streams v:0 -show_entries stream=nb_read_frames -of default=nokey=1:noprint_wrappers=1 $FILE_PATH
# Print frame info
$FFPROBE -v info -show_format -show_frames -print_format json $FILE_PATH