#!/bin/sh
# Download live video script
# Author:	Max.Chiu

FFMPEG=/Users/max/Documents/tools/build/bin/ffmpeg

VIDEO_URL="https://r2---sn-i3b7knse.googlevideo.com/videoplayback?expire=1563356280&ei=GJguXaH6F9KQqAGNn4zoDw&ip=103.29.140.19&id=o-AHAEPQ5jUvVWEjfhSThDm9lAB79xeJ_iNje9mN-XhiOt&itag=397&aitags=133%2C134%2C135%2C136%2C137%2C160%2C242%2C243%2C244%2C247%2C248%2C278%2C394%2C395%2C396%2C397%2C398&source=youtube&requiressl=yes&mm=31%2C29&mn=sn-i3b7knse%2Csn-i3beln76&ms=au%2Crdu&mv=m&mvi=1&pl=25&initcwndbps=473750&mime=video%2Fmp4&gir=yes&clen=10422271&dur=224.880&lmt=1558553062273478&mt=1563334559&fvip=6&keepalive=yes&c=WEB&txp=5531432&sparams=expire%2Cei%2Cip%2Cid%2Caitags%2Csource%2Crequiressl%2Cmime%2Cgir%2Cclen%2Cdur%2Clmt&lsparams=mm%2Cmn%2Cms%2Cmv%2Cmvi%2Cpl%2Cinitcwndbps&lsig=AHylml4wRQIgLc3iu2C9VYxwOsSyY6Lh7DVaYUCrn_SZAoEjRSDUq7YCIQDt9G6L0WUbU1-NGY-xucNw3KSmIU3LEg8uRTgEzUFDww%3D%3D&alr=yes&sig=ALgxI2wwRAIgXYhQwyH2203Gzr56RvXlG-M6vUs_8dosGM9i714quLQCID08oW0u0pgHosn1OunhU_pmozK1-_8zjh-dvp5inlCk&cpn=hn_K-R4mg_9DopMc&cver=2.20190716&rn=14&rbuf=0"
AUDIO_URL="https://r2---sn-i3b7knse.googlevideo.com/videoplayback?expire=1563355755&ei=C5YuXcz2Bs7EgQPmlLa4DQ&ip=103.29.140.19&id=o-ANXIs_rwSRu3HZ1ilGqu-D2XBpaUgD_DqdgS15nXpt0d&itag=251&source=youtube&requiressl=yes&mm=31%2C29&mn=sn-i3b7knse%2Csn-i3beln76&ms=au%2Crdu&mv=m&mvi=1&pl=25&initcwndbps=1055000&mime=audio%2Fwebm&gir=yes&clen=3939690&dur=224.901&lmt=1555340298133086&mt=1563334066&fvip=6&keepalive=yes&c=WEB&txp=5531432&sparams=expire%2Cei%2Cip%2Cid%2Citag%2Csource%2Crequiressl%2Cmime%2Cgir%2Cclen%2Cdur%2Clmt&lsparams=mm%2Cmn%2Cms%2Cmv%2Cmvi%2Cpl%2Cinitcwndbps&lsig=AHylml4wRQIgHzTM-3vrYKQ2PDKhcfLBo3shnzgyRzmOl16ht0z2E2ACIQDRbgPcUIy8kuG4a0o5W4HVtiGvwXy1gZv9sm9fynLutg%3D%3D&alr=yes&sig=ALgxI2wwRAIgaqucRgjp7gWF4cty9zVqdrAx7CpItY5R3HatTIQBo8sCIDBDmMjiLNO2sKrJkZqHupFe57zC1wjQIW_EEjP_zg1n&cpn=BtJEaXE-AFaLHfxq&cver=2.20190716&rn=7&rbuf=0"
VIDEO_NAME=video.mp4
AUDIO_NAME=audio.mp4

# Combine mp4
$FFMPEG -i $VIDEO_URL -y ./$VIDEO_NAME
#$FFMPEG -i $AUDIO_URL -y ./$AUDIO_NAME

$FFMPEG -i $VIDEO_NAME -i $AUDIO_NAME -y output.mp4