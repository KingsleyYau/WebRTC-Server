#!/bin/sh
# Pakcage mediasever script
# Author: Max.Chiu
# Date: 2019/11/11

CUR_DIR=$(dirname $(readlink -f "$0"))

TMP=$CUR_DIR/tmp
mkdir -p $TMP
mkdir -p package
rm -rf package/*

VERSION=`cat version.json | jq -c '.version' `
VERSION=`echo $VERSION | sed s/\"//g`
echo -e "VERSION:[\033[32m$VERSION\033[0m]"

ENVS=( \
local_camchat_201 local_camchat_202 \
product_cam \
local demo_jp demo_eu \
product_sg product_au product_eu product_us product_vn product_br \
)

function package_tar {
  echo -e "############## package_tar ##############"
	ENV=$1
	TMP=$2
	
	DEST=$TMP/$ENV/file
	mkdir -p $DEST
	
 	# Copy Install/Update Script Files
	cp -rf bin/install.sh $TMP/$ENV/ || return 1
		
	# Replace Custom Install/Update
	if [ -f "conf/$ENV/bin/install.sh" ]; then
	  cp -rf conf/$ENV/bin/install.sh $TMP/$ENV/ || return 1
	fi
	
	# Copy Version File
	cp -rf version.json $DEST/version.json || return 1
	
	# Copy Executable Files
	mkdir -p $DEST/bin/ || return 1
	cp -rf build/bin/ffmpeg $DEST/bin/ || return 1
	cp -rf build/bin/mediaserver $DEST/bin/ || return 1
	cp -rf build/bin/turnadmin $DEST/bin/ || return 1
	cp -rf build/bin/turnserver $DEST/bin/ || return 1
  cp -rf build/bin/wscat $DEST/bin/ || return 1
  
	# Copy Config Files
	mkdir -p $DEST/etc/ || return 1
	cp -rf conf/$ENV/etc $DEST/ || return 1
	
	# Copy Script Files
	mkdir -p $DEST/script/ || return 1
	cp -rf script $DEST/ || return 1
	
	# Copy Var Files
	mkdir -p $DEST/var/ || return 1
	#cp -rf conf/$ENV/var $DEST/ || return 1
	
	# Replace Custom Config
	if [ -d "conf/$ENV/etc" ]; then
	  cp -rf conf/$ENV/etc $DEST/ || return 1
	fi
	
	# Replace Custom Script
	if [ -d "conf/$ENV/script" ]; then
	  cp -rf conf/$ENV/script $DEST/ || return 1
	fi
	
	# Clean index files
	find $TMP/$ENV -name ".DS_Store" | xargs rm -rf || return 1
	
	return 0
}

for ENV in ${ENVS[@]};do
	echo -e "############## Packaging [\033[33m$ENV\033[0m] ##############"
	package_tar $ENV $TMP
	
	if [ $? == 0 ]; then
		# Start packaging
	  cd $TMP
	  PACKAGE_FILE="${ENV}-${VERSION}.tar.gz"
	  tar zcvf ../package/$PACKAGE_FILE $ENV || exit 1
	  cd -
	  echo -e "############## Packaging [\033[33m$ENV\033[0m] [\033[32mOK\033[0m] ##############"
	else
	  echo -e "############## Packaging [\033[33m$ENV\033[0m] [\033[31mFail\033[0m] ##############"
	fi

  rm -rf $TMP
	echo "# Package file: package/$PACKAGE_FILE"
	
	echo -e "############## Packaging [\033[33m$ENV\033[0m] update ##############"
	mkdir -p $TMP
	./package_update.sh $ENV $TMP
	if [ $? == 0 ]; then
	  cd $TMP
	  PACKAGE_FILE="${ENV}-update-${VERSION}.tar.gz"
	  tar zcvf ../package/$PACKAGE_FILE $ENV || exit 1
	  cd -
	  echo -e "############## Packaging [\033[33m$ENV\033[0m] update [\033[32mOK\033[0m] ##############"
	else
	  echo -e "############## Packaging [\033[33m$ENV\033[0m] update [\033[31mFail\033[0m] ##############"
	fi
	echo "# Package update file: package/$PACKAGE_FILE"
	
	echo ""
done