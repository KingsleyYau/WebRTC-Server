#!/bin/sh
# Create certificate shell
# Author: Max.Chiu
# Date: 2019/08/13

# Create certificate with interactive model
#openssl req -x509 -nodes -newkey rsa:1024 -keyout webrtc_dtls.key -out webrtc_dtls.crt

# Create certificate with non-interactive model
openssl req -x509 -nodes -newkey rsa:1024 -keyout webrtc_dtls.key -out webrtc_dtls.crt -subj /C=CN/ST=Hongkong/L=Hongkong/O=EST/OU=EST/CN=mediaserver/emailAddress=kingsleyyau@gmail.com

# Create private key
#openssl genrsa -passout pass:1234 -des3 -out webrtc_dtls.key 1024
# Create certificate with specific private key
#openssl req -x509 -passin pass:1234 -key webrtc_dtls.key -out webrtc_dtls.crt -subj /C=CN/ST=Hongkong/L=Hongkong/O=EST/OU=EST/CN=mediaserver/emailAddress=kingsleyyau@gmail.com