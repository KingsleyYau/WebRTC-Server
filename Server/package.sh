#!/bin/sh
# Pakcage mediasever script
# Author: Max.Chiu
# Date: 2019/11/11

mkdir -p tmp
rm -rf package/*

VERSION=`cat version.json | jq -c '.version' `
VERSION=`echo $VERSION | sed s/\"//g`
echo "VERSION:$VERSION"

ENVS=( \
local demo_jp demo_eu \
product_sg product_au product_eu product_us product_th product_vn product_br \
)

function packet_tar {
 	# Copy Install/Update Script Files
	cp -rf bin/ tmp/$ENV/ || return 1
	
	# Copy Executable Files
	mkdir -p tmp/$ENV/bin/ || return 1
	cp -rf build/bin/ffmpeg tmp/$ENV/bin/ || return 1
	cp -rf build/bin/mediaserver tmp/$ENV/bin/ || return 1
	cp -rf build/bin/turnadmin tmp/$ENV/bin/ || return 1
	cp -rf build/bin/turnserver tmp/$ENV/bin/ || return 1
	
	# Copy Config Files
	mkdir -p tmp/$ENV/etc/ || return 1
	cp -rf conf/$ENV/etc tmp/$ENV/ || return 1
	mkdir -p tmp/$ENV/script/ || return 1
	cp -rf script tmp/$ENV/ || return 1
	mkdir -p tmp/$ENV/var/ || return 1
	#cp -rf conf/$ENV/var tmp/$ENV/ || return 1
	
	# Clean index files
	find tmp -name ".DS_Store" | xargs rm -rf || return 1
	
	return 0
}

for ENV in ${ENVS[@]};do
	echo -e "############## Packaging [\033[33m$ENV\033[0m] ##############"
	packet_tar
	
	if [ $? == 0 ]; then
		# Start packaging
    mkdir -p package
	  cd tmp
	  PACKAGE_FILE="${ENV}-${VERSION}.tar.gz"
	  tar zcvf ../package/$PACKAGE_FILE $ENV
	  cd -
	  echo -e "############## Packaging [\033[33m$ENV\033[0m] [\033[32mOK\033[0m] ##############"
	else
	  echo -e "############## Packaging [\033[33m$ENV\033[0m] [\033[31mFail\033[0m] ##############"
	fi

	echo "# Package file: package/$PACKAGE_FILE"
	echo ""
done

rm -rf tmp