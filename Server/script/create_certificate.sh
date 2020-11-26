#!/bin/sh
# Create certificate shell
# Author: Max.Chiu
# Date: 2019/08/13

# Create output directory
mkdir output

# Create CA key
#openssl genrsa -out output/private/cakey.pem 2048
# Create CA certificate
#openssl req -new -x509 -nodes -key output/private/cakey.pem -days 824 -sha256 -out output/private/cacert.crt -subj /C=CN/ST=Hongkong/L=Hongkong/O=EST/OU=EST/CN=EST/emailAddress=max.chiu@qpidnetwork.com -extensions v3_ca


# Create User key
openssl genrsa -out output/server.key 2048
# Create User request
openssl req -new -key output/server.key -config cert.cnf -out output/server.csr
# Signed User certificate
openssl x509 -req -in output/server.csr -CA output/private/cacert.crt -CAkey output/private/cakey.pem -CAcreateserial -out output/server.crt -days 824 -sha256 -extfile cert.cnf -extensions req_ext


# Verify User certificate
openssl verify -CAfile output/private/cacert.crt output/server.crt 
# Show User certificate info
openssl x509 -text -noout -in output/server.crt
# Show User key info
#openssl rsa -text -noout -in output/server.key
# Show User csr info
#openssl req -text -noout -in output/server.csr
# Check remote domain certificate
#openssl s_client -connect demo-stream2.charmlive.com:9082 -CAfile output/private/cacert.crt


# Create certificate with interactive model
#openssl req -x509 -nodes -newkey rsa:1024 -keyout output/webrtc_dtls.key -out output/webrtc_dtls.crt
# Create certificate with non-interactive model
#openssl req -x509 -days 824 -nodes -newkey rsa:1024 -keyout output/webrtc_dtls.key -out output/webrtc_dtls.crt -subj /C=CN/ST=Hongkong/L=Hongkong/O=mediaserver/OU=mediaserver/CN=mediaserver/emailAddress=max.chiu@qpidnetwork.com
#openssl req -x509 -days 824 -nodes -newkey rsa:1024 -keyout output/server.key -out output/server.crt -subj /C=CN/ST=Hongkong/L=Hongkong/O=mediaserver/OU=mediaserver/CN=mediaserver/emailAddress=max.chiu@qpidnetwork.com

# Create private key with password
#openssl genrsa -passout pass:123456 -des3 -out output/webrtc_dtls.key 1024
# Create certificate with specific private key with password
#openssl req -x509 -passin pass:123456 -key webrtc_dtls.key -out output/webrtc_dtls.crt -subj /C=CN/ST=Hongkong/L=Hongkong/O=EST/OU=EST/CN=mediaserver/emailAddress=max.chiu@qpidnetwork.com