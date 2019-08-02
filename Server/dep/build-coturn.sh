#!/bin/sh
# coturn build script for linux
# Author:	Max.Chiu
# Description: asm

# Config version
VERSION=4.5.1.1

function configure_prefix {
	export PREFIX=$(pwd)/build
}

function build_sqlite {
	SQLITE="sqlite"
	echo "# Start building $SQLITE"
	
	cd $SQLITE
	./configure \
				--prefix=$PREFIX \
				--enable-static \
				--disable-shared \
				|| exit 1   		
    		
	make clean || exit 1
	make || exit 1
	make install || exit 1
	
	cd ..
	echo "# Build $SQLITE finish"
}

function build_libevent {
	LIBEVENT="$libevent-2.1.11-stable"
	echo "# Start building $LIBEVENT"
	
	cd $LIBEVENT
	./configure \
				--prefix=$PREFIX \
				--enable-static \
				--disable-shared \
				|| exit 1   		
    		
	make clean || exit 1
	make || exit 1
	make install || exit 1
	
	cd ..
	echo "# Build $LIBEVENT finish"
}

function build_openssl {
	OPENSSL="openssl-1.1.1c"
	echo "# Start building $OPENSSL"

	cd $OPENSSL
	
	./configure \
				--prefix=$PREFIX \
				--enable-static \
				--disable-shared \
				|| exit 1

	make clean || exit 1
	make || exit 1
	make install || exit 1
	
	cd ..
	echo "# Build $OPENSSL finish"
}

function build_coturn {
	COTURN="coturn-4.5.1.1"
	echo "# Start building $COTURN"
	
	cd $COTURN

	# build
	./configure \
						--prefix="$PREFIX" \
    				|| exit 1
    				
						
	make clean || exit 1
	make || exit
	make install || exit 1

	cd ..
	echo "# Build $COTURN finish"
}

# Start Build

echo "# Starting building..."

configure_prefix || exit 1
build_sqlite || exit 1
build_libevent || exit 1
build_openssl || exit 1
build_coturn || exit 1