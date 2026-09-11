#!/bin/sh
# MediaServer dependence script
# Author: Max.Chiu

echo "# Install dependent tools ..."
DEVEPLOE=""
if [ "$1" != "" ];then
  DEVEPLOE="$1"
fi

if [ "$DEVEPLOE" == "develope" ];then
  sudo yum install -y automake libtool gcc-c++ perl cmake
  cd dep
  # Extract Dependence
  find . -name "*.tar.gz" | xargs -I {} tar zxvf {}
  cd -
fi

sudo dnf config-manager --set-enabled crb
sudo yum install -y epel-release
sudo yum install -y sysstat gdb telnet vim mtr zlib-devel gtk-doc glib2-devel boost-devel libidn2-devel libselinux-devel libuuid-devel