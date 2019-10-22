#!/bin/sh
# Pakcage mediasever script
# Author:	Max.Chiu

mkdir -p tmp
rm -rf package/*

ENVS=(local demo product)
for ENV in ${ENVS[@]};do
	echo "############## Start packaging [$ENV] ##############"
	
	# Copy Executable Files
	mkdir -p tmp/$ENV/bin/
	cp -rf build/bin/ffmpeg tmp/$ENV/bin/
	cp -rf build/bin/mediaserver tmp/$ENV/bin/
	cp -rf build/bin/turnadmin tmp/$ENV/bin/
	cp -rf build/bin/turnserver tmp/$ENV/bin/
	
	# Copy Config Files
	mkdir -p tmp/$ENV/etc/
	cp -rf conf/$ENV/etc tmp/$ENV/etc/
	mkdir -p tmp/$ENV/script/
	cp -rf conf/$ENV/script tmp/$ENV/script/
	mkdir -p tmp/$ENV/var/
	cp -rf conf/$ENV/var tmp/$ENV/var/
	
	mkdir -p package
	cd tmp
	tar zcvf ../package/$ENV.tar.gz $ENV
	cd -
	
	echo "############## Finish packaging [$ENV] ##############"
	echo "# Package file: package/$ENV.tar.gz"
	echo ""
done

rm -rf tmp