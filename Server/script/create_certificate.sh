#!/bin/sh
# Create certificate shell
# Author: Max.Chiu
# Date: 2019/08/13

# Create output directory
mkdir output

rm output/index.txt -rf
touch output/index.txt
rm output/index -rf
echo 01 > output/serial

# Create CA key
#openssl genrsa -out output/private/cakey.pem 1024
# Create CA certificate
#openssl req -new -x509 -days 3650 -key output/private/cakey.pem -out output/private/cacert.pem -subj /C=CN/ST=Hongkong/L=Hongkong/O=mediaserver/OU=mediaserver/CN=mediaserver/emailAddress=max.chiu@qpidnetwork.com -extensions v3_ca
# Create User key
#openssl genrsa -out output/server.key 1024
# Create User request
#openssl req -new -key output/server.key -out output/server.req -subj /C=CN/ST=Hongkong/L=Hongkong/O=mediaserver/OU=mediaserver/CN=mediaserver/emailAddress=max.chiu@qpidnetwork.com
# Signed User certificate
#openssl ca -in output/server.req -days 3650 -out output/server.crt -config openssl.cnf -extensions v3_req

# Create certificate with interactive model
#openssl req -x509 -nodes -newkey rsa:1024 -keyout output/webrtc_dtls.key -out output/webrtc_dtls.crt
# Create certificate with non-interactive model
#openssl req -x509 -days 3650 -nodes -newkey rsa:1024 -keyout output/webrtc_dtls.key -out output/webrtc_dtls.crt -subj /C=CN/ST=Hongkong/L=Hongkong/O=mediaserver/OU=mediaserver/CN=mediaserver/emailAddress=max.chiu@qpidnetwork.com
openssl req -x509 -days 3650 -nodes -newkey rsa:1024 -keyout output/server.key -out output/server.crt -subj /C=CN/ST=Hongkong/L=Hongkong/O=mediaserver/OU=mediaserver/CN=mediaserver/emailAddress=max.chiu@qpidnetwork.com

# Create private key with password
#openssl genrsa -passout pass:123456 -des3 -out output/webrtc_dtls.key 1024
# Create certificate with specific private key with password
#openssl req -x509 -passin pass:123456 -key webrtc_dtls.key -out output/webrtc_dtls.crt -subj /C=CN/ST=Hongkong/L=Hongkong/O=EST/OU=EST/CN=mediaserver/emailAddress=max.chiu@qpidnetwork.com