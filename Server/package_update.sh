#!/bin/sh
# Pakcage update mediasever script
# Author: Max.Chiu
# Date: 2019/11/11

function package_update_tar {
  echo -e "############## package_update_tar ##############"
	ENV=$1
	TMP=$2

  mkdir -p $TMP/$ENV/

 	# Copy Install/Update Script Files
	cp -rf bin/update.sh $TMP/$ENV/ || return 1
	
	# Copy Version File
	cp -rf version.json $TMP/$ENV/ || return 1
	
	# Copy Executable Files
	mkdir -p $TMP/$ENV/bin/ || return 1
	cp -rf build/bin/mediaserver $TMP/$ENV/bin/ || return 1

	# Copy Config Files
	mkdir -p $TMP/$ENV/etc/ || return 1
	cp -rf conf/$ENV/etc/mediaserver_camshare.config $TMP/$ENV/etc/ || return 1
	
	mkdir -p $TMP/$ENV/script/ || return 1
	cp -rf script/start_mediaserver_camshare.sh $TMP/$ENV/script || return 1
	cp -rf script/stop_mediaserver_camshare.sh $TMP/$ENV/script || return 1
	cp -rf script/restart_test_service.sh $TMP/$ENV/script || return 1
	cp -rf script/stop_test_service.sh $TMP/$ENV/script || return 1
	
	# Clean index files
	find tmp -name ".DS_Store" | xargs rm -rf || return 1
	
	return 0
}

package_update_tar $1 $2