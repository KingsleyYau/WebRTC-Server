#!/bin/sh
# Pakcage update mediasever script
# Author: Max.Chiu
# Date: 2019/11/11

function package_update_tar {
  echo -e "############## package_update_tar ##############"
  ENV=$1
  TMP=$2

  DEST=$TMP/$ENV/file
  mkdir -p $DEST

  # Copy Install/Update Script Files
  cp -rf bin/update.sh $TMP/$ENV/ || return 1
	# Replace Custom Install/Update
	if [ -f "conf/$ENV/bin/update.sh" ]; then
	  cp -rf conf/$ENV/bin/update.sh $TMP/$ENV/ || return 1
	fi
	
  # Copy Version File
  cp -rf version.json $DEST || return 1
	
  # Copy Executable Files
  mkdir -p $DEST/bin/ || return 1
  cp -rf build/bin/mediaserver $DEST/bin/ || return 1
  #cp -rf build/bin/turnserver $DEST/bin/ || return 1
  #cp -rf build/bin/ffmpeg $DEST/bin/ || return 1
  #cp -rf build/bin/wscat $DEST/bin/ || return 1

  # Copy Config Files
  #mkdir -p $DEST/etc/ || return 1
  #cp -rf conf/$ENV/etc/mediaserver.config $DEST/etc/ || return 1
  #if [ -f "conf/$ENV/etc/mediaserver_test.config" ]; then
  #  cp -rf conf/$ENV/etc/mediaserver_test.config $DEST/etc/ || return 1
  #fi
  #if [ -f "conf/$ENV/etc/mediaserver_camshare.config" ]; then
  #  cp -rf conf/$ENV/etc/mediaserver_camshare.config $DEST/etc/ || return 1
  #fi
  
  mkdir -p $DEST/script/ || return 1
  #cp -rf script/check_status.sh $DEST/script/ || return 1
  #cp -rf script/check_status_detail.sh $DEST/script/ || return 1
  #cp -rf script/rtp2rtmp.sh $DEST/script/ || return 1
  #cp -rf script/rtp2rtmp_camshare.sh $DEST/script/ || return 1
  #cp -rf script/rtmp2rtp.sh $DEST/script/ || return 1
  #cp -rf script/stop_turnserver.sh $DEST/script/ || return 1
  #cp -rf script/stop_mediaserver.sh $DEST/script/ || return 1
  #cp -rf script/stop_mediaserver_test.sh $DEST/script/ || return 1
  #cp -rf script/start_mediaserver_camshare.sh $DEST/script/ || return 1
  #cp -rf script/stop_mediaserver_camshare.sh $DEST/script/ || return 1
  #cp -rf script/restart_test_service.sh $DEST/script/ || return 1
  #cp -rf script/stop_test_service.sh $DEST/script || return 1
  #cp -rf script/restart_all_service.sh $DEST/script/ || return 1
  #cp -rf script/stop_all_service.sh $DEST/script/ || return 1
  #cp -rf script/deamon.sh $DEST/script/ || return 1
  #cp -rf script/dump_thread_bt.init $DEST/script/ || return 1
  #cp -rf script/restart_by_deamon.sh $DEST/script/ || return 1
  #cp -rf script/stop_by_deamon.sh $DEST/script/ || return 1
	
  # Replace Custom Script
  #if [ -f "conf/$ENV/script/deamon.sh" ]; then
  #  cp -rf conf/$ENV/script/deamon.sh $DEST/script/ || return 1
  #fi
  #if [ -f "conf/$ENV/script/restart_by_deamon.sh" ]; then
  #  cp -rf conf/$ENV/script/restart_by_deamon.sh $DEST/script/ || return 1
  #fi
	#if [ -f "conf/$ENV/script/stop_by_deamon.sh" ]; then
  #  cp -rf conf/$ENV/script/stop_by_deamon.sh $DEST/script/ || return 1
  #fi
  #if [ -f "conf/$ENV/script/stop_all_service.sh" ]; then
  #  cp -rf conf/$ENV/script/stop_all_service.sh $DEST/script/ || return 1
  #fi
  #if [ -f "conf/$ENV/script/restart_all_service.sh" ]; then
  #  cp -rf conf/$ENV/script/restart_all_service.sh $DEST/script/ || return 1
  #fi
  #if [ -f "conf/$ENV/script/restart_test_service.sh" ]; then
  #  cp -rf conf/$ENV/script/restart_test_service.sh $DEST/script/ || return 1
  #fi    
  #if [ -f "conf/$ENV/script/stop_test_service.sh" ]; then
  #  cp -rf conf/$ENV/script/stop_test_service.sh $DEST/script/ || return 1
  #fi     
  # Clean index files
  find tmp -name ".DS_Store" | xargs rm -rf || return 1
	
  return 0
}

package_update_tar $1 $2