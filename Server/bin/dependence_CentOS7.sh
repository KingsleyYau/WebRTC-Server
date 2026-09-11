#!/bin/sh
# WebRTC MediaServer compile whole script
# Author: Max.Chiu

echo "# Install dependent tools ..."
sudo yum install -y automake libtool gcc-c++ zlib-devel gtk-doc glib2-devel boost-devel libidn-devel libselinux-devel libuuid-devel

#wget http://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
#sudo rpm -ivh epel-release-latest-7.noarch.rpm
#sudo yum install -y jq cmake3
#rm epel-release-latest-7.noarch.rpm -f