#!/bin/sh
# extract package script for linux
# Author:	Max.Chiu
# Description: asm

PACKAGES=`find . -name "*.tar.gz"`
echo $PACKAGES

for PACKAGE in ${PACKAGES[@]};do
	tar zxvf $PACKAGE
done

echo "# Extract packages done."