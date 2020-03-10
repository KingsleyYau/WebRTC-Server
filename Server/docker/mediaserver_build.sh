#!/bin/sh
# Docker mediasever script
# Author: Max.Chiu
# Date: 2020/03/04

CUR_DIR=$(dirname $(readlink -f "$0"))
SRC_DIR=$CUR_DIR/..

TMP=$CUR_DIR/tmp
mkdir -p $TMP

DOCKERFILE=$CUR_DIR/Dockerfile

VERSION=`cat $SRC_DIR/version.json | jq -c '.version' `
VERSION=`echo $VERSION | sed s/\"//g`
echo -e "VERSION:[\033[32m$VERSION\033[0m]"

ENVS=( \
docker
)

function build_mediaserver {
	ENV=$1
	TMP=$2
	
	DEST=$TMP/$ENV
	mkdir -p $DEST
		
	# Copy Version File
	cp -rf $SRC_DIR/version.json $DEST/version.json || return 1
	
	# Copy Executable Files
	mkdir -p $DEST/bin/ || return 1
	cp -rf $SRC_DIR/build/bin/ffmpeg $DEST/bin/ || return 1
	cp -rf $SRC_DIR/build/bin/mediaserver $DEST/bin/ || return 1
	cp -rf $SRC_DIR/build/bin/turnadmin $DEST/bin/ || return 1
	cp -rf $SRC_DIR/build/bin/turnserver $DEST/bin/ || return 1
	
	# Copy Config Files
	#mkdir -p $DEST/etc/ || return 1
	cp -rf $SRC_DIR/conf/$ENV/etc $DEST/ || return 1
	
	# Copy Script Files
	#mkdir -p $DEST/script/ || return 1
	cp -rf $SRC_DIR/script $DEST/ || return 1
	
	# Copy Var Files
	mkdir -p $DEST/var/ || return 1
	#cp -rf $SRC_DIR/conf/$ENV/var $DEST/ || return 1
	
	# Copy Run Files
	mkdir -p $DEST/run/ || return 1

	# Clean index files
	find $TMP/$ENV -name ".DS_Store" | xargs rm -rf || return 1
	
	return 0
}

for ENV in ${ENVS[@]};do
	echo -e "############## Packaging [\033[33m$ENV\033[0m] ##############"
	build_mediaserver $ENV $TMP
	
	if [ $? == 0 ]; then
		# Start packaging
	  cd $TMP/$ENV
	  docker image build -t mediaserver -f $DOCKERFILE .
	  cd -
	  echo -e "############## Packaging [\033[33m$ENV\033[0m] [\033[32mOK\033[0m] ##############"
	else
	  echo -e "############## Packaging [\033[33m$ENV\033[0m] [\033[31mFail\033[0m] ##############"
	fi

	echo ""
done

rm -rf $TMP

result=$(docker ps -a | grep mediaserver | awk -F ' ' '{print $1}')
if [ "$result" != "" ];then
  echo -e "# Stop container [\033[33m$result\033[0m]"
  docker stop $result
fi

result=$(docker images | grep "<none>" | awk -F ' ' '{print $3}')
if [ "$result" != "" ];then
  echo -e "# Remove old images [\033[33m$result\033[0m]"
  docker rmi $result --force
fi