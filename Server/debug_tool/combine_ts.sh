FFMPEG=/Users/max/Documents/tools/build/bin/ffmpeg

$FFMPEG -i `echo -e "concat:\c";for i in {0..22};do echo -e "./ts/$i.ts|\c";done` -acodec copy -absf aac_adtstoasc -vcodec copy out.mp4