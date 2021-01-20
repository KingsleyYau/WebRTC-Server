#!/bin/sh
# ffmpeg build script for linux
# Author:	Max.Chiu
# Description: asm

# Config version
VERSION=4.3.1

BUILD_PATH="$1"
if [ "$BUILD_PATH" == "" ]; then
  echo "# BUILD_PATH must be set, like: /root/mediaserver/build"
fi

NOCLEAN="$2"
if [ "$NOCLEAN" == "noclean" ]; then
	echo "# Build ffmpeg without clean"
else
  echo "# Build ffmpeg with clean"
  NOCLEAN=""
fi

function configure_prefix {
  export PREFIX=$BUILD_PATH
  export PATH="$PREFIX/bin:$PATH"

  export PKG_CONFIG_LIBDIR=$PREFIX/lib/pkgconfig
  export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig

  export EXTRA_CFLAGS="-I$PREFIX/include $EXTRA_CFLAGS" 
  export EXTRA_LDFLAGS="-L$PREFIX/lib $EXTRA_LDFLAGS -lm -pthread -ldl"
}

function build_opus {
  SUB_LIB="opus-1.3.1"
  echo "# Start building $SUB_LIB"
	
  cd $SUB_LIB
  if [ "$NOCLEAN" != "noclean" -o ! -f "Makefile" ]; then 
    ./configure \
      --prefix=$PREFIX \
      --enable-static \
      --disable-shared \
      || exit 1   		
  fi
		
  if [ "$NOCLEAN" != "noclean" ]; then
    make clean || exit 1
  fi

  make || exit 1
  make install || exit 1

  cd ..
  echo "# Build $SUB_LIB finish"
}

function build_x264 {
  SUB_LIB="x264"
  echo "# Start building $SUB_LIB"
	
  cd $SUB_LIB
  if [ "$NOCLEAN" != "noclean" -o ! -f "Makefile" ]; then
    ./configure \
      --prefix=$PREFIX \
      --enable-static \
      --enable-pic \
      --disable-cli \
      --disable-asm \
      || exit 1   		
  fi

  if [ "$NOCLEAN" != "noclean" ]; then
    make clean || exit 1
	fi

  make || exit 1
  make install || exit 1
	
  cd ..
  echo "# Build $SUB_LIB finish"
}

function build_fdk_aac {
  SUB_LIB="fdk-aac-0.1.5"

  echo "# Start building $SUB_LIB"

  cd $SUB_LIB
  if [ "$NOCLEAN" != "noclean" -o ! -f "Makefile" ]; then
    ./configure \
      --prefix=$PREFIX \
      --enable-static \
      --disable-shared \
      --with-pic \
      || exit 1
	fi

  if [ "$NOCLEAN" != "noclean" ]; then
   make clean || exit 1
  fi

  make || exit 1
  make install || exit 1

  cd ..
  echo "# Build $SUB_LIB finish"
}

function build_yasm {
  SUB_LIB="yasm-1.3.0"
  echo "# Start building $SUB_LIB"
    
  cd $SUB_LIB
  if [ "$NOCLEAN" != "noclean" -o ! -f "Makefile" ]; then
    ./configure \
      --prefix=$PREFIX \
      || exit 1
  fi

  if [ "$NOCLEAN" != "noclean" ]; then
    make clean || exit 1
  fi

  make || exit 1
  make install || exit 1

  cd ..
  echo "# Build $SUB_LIB finish"
}


function build_ffmpeg {
  FFMPEG="ffmpeg-$VERSION"
  echo "# Start building $FFMPEG"
  
  cd $FFMPEG

  # build
  if [ "$NOCLEAN" != "noclean" -o ! -f "Makefile" ]; then
    ./configure \
      --prefix="$PREFIX" \
      --extra-cflags="$EXTRA_CFLAGS" \
      --extra-ldflags="$EXTRA_LDFLAGS" \
      --disable-shared \
      --enable-static \
      --enable-gpl \
      --enable-openssl \
      --enable-libopus \
      --enable-libx264 \
      --enable-nonfree \
      --enable-libfdk-aac \
      --enable-version3 \
      --disable-iconv \
      --disable-outdevs \
      --disable-ffprobe \
      --enable-encoder=libx264 \
      --enable-decoder=h264 \
      --enable-demuxer=h264 \
      --enable-parser=h264 \
      --enable-encoder=libfdk_aac \
      --enable-decoder=libfdk_aac \
      --enable-encoder=libopus \
      --enable-decoder=libopus \
      || exit 1
  fi  				
  					
  if [ "$NOCLEAN" != "noclean" ]; then
    make clean || exit 1
  fi
	
	make || exit
  make install || exit 1

  cd ..
  echo "# Build $FFMPEG finish"
}

# Start Build

echo "# Starting building..."

configure_prefix || exit 1
build_fdk_aac || exit 1
build_x264 || exit 1
build_opus || exit 1
build_yasm || exit 1
build_ffmpeg || exit 1