#!/bin/sh
# Pakcage mediasever script
# Author: Max.Chiu
# Date: 2019/11/11

mkdir -p tmp
rm -rf package/*

VERSION=`cat version.json | jq -c '.version' `
VERSION=`echo $VERSION | sed s/\"//g`
echo "VERSION:$VERSION"

ENVS=(local demo product)
for ENV in ${ENVS[@]};do
	echo "############## Start packaging [$ENV] ##############"
	# Copy Install/Update Script Files
	cp -rf bin/ tmp/$ENV/
	
	# Copy Executable Files
	mkdir -p tmp/$ENV/bin/
	cp -rf build/bin/ffmpeg tmp/$ENV/bin/
	cp -rf build/bin/mediaserver tmp/$ENV/bin/
	cp -rf build/bin/turnadmin tmp/$ENV/bin/
	cp -rf build/bin/turnserver tmp/$ENV/bin/
	
	# Copy Config Files
	mkdir -p tmp/$ENV/etc/
	cp -rf conf/$ENV/etc tmp/$ENV/
	mkdir -p tmp/$ENV/script/
	cp -rf conf/$ENV/script tmp/$ENV/
	mkdir -p tmp/$ENV/var/
	cp -rf conf/$ENV/var tmp/$ENV/
	
	mkdir -p package
	cd tmp
	PACKAGE_FILE="${ENV}-${VERSION}.tar.gz"
	tar zcvf ../package/$PACKAGE_FILE $ENV
	cd -
	
	echo "############## Finish packaging [$ENV] ##############"
	echo "# Package file: package/$PACKAGE_FILE"
	echo ""
done

rm -rf tmp