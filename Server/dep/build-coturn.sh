#!/bin/sh
# coturn build script for linux
# Author:	Max.Chiu
# Description: asm

# Config version
VERSION=4.5.1.1

function configure_prefix {
	export PREFIX=$(pwd)/build
	
	export PKG_CONFIG_LIBDIR=$PREFIX/lib/pkgconfig
	export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig
}

function build_sqlite {
	SQLITE="sqlite"
	echo "# Start building $SQLITE"
	
	cd $SQLITE
	
	# build
	chmod +x configure
	./configure \
				--prefix=$PREFIX \
				--enable-static \
				--disable-shared \
				|| exit 1   		
    		
	make clean || exit 1
	make || exit 1
	make install || exit 1
	
	cd -
	echo "# Build $SQLITE finish"
}

function build_openssl {
	OPENSSL="openssl-1.1.1c"
	echo "# Start building $OPENSSL"

	cd $OPENSSL
	
	# build
	chmod +x config
	./config no-shared --prefix=$PREFIX \
				|| exit 1

	make clean || exit 1
	make || exit 1
	make install || exit 1
	
	cd -
	echo "# Build $OPENSSL finish"
}

function build_libevent {
	LIBEVENT="libevent-2.1.11-stable"
	echo "# Start building $LIBEVENT"
	
	cd $LIBEVENT
	
	# build
	export CFLAGS="$(pkg-config --cflags openssl)"
	export LDFLAGS="$(pkg-config --libs openssl) -ldl"
	
	chmod +x configure
	./configure \
				--prefix=$PREFIX \
				--enable-static \
				--disable-shared \
				|| exit 1   		
    		
	make clean || exit 1
	make || exit 1
	make install || exit 1
	
	cd -
	echo "# Build $LIBEVENT finish"
}

function build_coturn {
	COTURN="coturn-4.5.1.1"
	echo "# Start building $COTURN"
	
	cd $COTURN

	# build
	export CFLAGS="$(pkg-config --cflags openssl)"
	export LDFLAGS="$(pkg-config --libs openssl) -ldl"
	export LDFLAGS="$LDFLAGS $(pkg-config --libs libevent_core)"
	export LDFLAGS="$LDFLAGS $(pkg-config --libs libevent_extra)"
	export LDFLAGS="$LDFLAGS $(pkg-config --libs libevent_openssl)"
	export LDFLAGS="$LDFLAGS $(pkg-config --libs libevent_pthreads)"
	
	chmod +x configure
	./configure \
						--prefix="$PREFIX" \
    				|| exit 1
    				
						
	make clean || exit 1
	make || exit
	make install || exit 1

	cd -
	echo "# Build $COTURN finish"
}

# Start Build

echo "# Starting building..."

configure_prefix || exit 1
#build_sqlite || exit 1
#build_openssl || exit 1
#build_libevent || exit 1
build_coturn || exit 1