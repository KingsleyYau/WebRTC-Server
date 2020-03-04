#!/bin/sh
# Docker install script
# Author: Max.Chiu
# Date: 2020/03/04

# https://docs.docker.com/install/linux/docker-ce/centos/

# Older versions of Docker were called docker or docker-engine. If these are installed, uninstall them, along with associated dependencies.
sudo yum remove docker \
  docker-client \
  docker-client-latest \
  docker-common \
  docker-latest \
  docker-latest-logrotate \
  docker-logrotate \
  docker-engine
  
# Install required packages. yum-utils provides the yum-config-manager utility, and device-mapper-persistent-data and lvm2 are required by the devicemapper storage driver.
sudo yum install -y yum-utils \
  device-mapper-persistent-data \
  lvm2
  
# Use the following command to set up the stable repository.
sudo yum-config-manager \
  --add-repo \
  https://download.docker.com/linux/centos/docker-ce.repo
  
# Install the latest version of Docker Engine - Community and containerd, or go to the next step to install a specific version.
sudo yum install docker-ce docker-ce-cli containerd.io

# Start Docker.
sudo systemctl start docker
sudo systemctl enable docker