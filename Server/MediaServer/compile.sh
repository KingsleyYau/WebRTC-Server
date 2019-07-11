#!/bin/sh

echo "# install dependent tools ..."
sudo yum install -y libidn-devel

# set ./bin/* executable
chmod -R +x ./bin/* || exit 1

# compile
if [ "$1" == "noclean" ]; then
  echo "# build CamShareServer without clean"
else
  echo "# bulid CamShareServer with clean"
  make clean
  chmod +x ./configure || exit 1
  ./configure || exit 1
fi
make || exit 1