Catalog
=================

   * [WebRTC流媒体网关服务器](#WebRTC流媒体网关服务器)
   		* [How it works](#how-it-works)
   		* [Compile & Installation](#compile-and-installation)
   		* [Sequence diagram](#sequence-diagram)
   		* [Test page](#test-page)
      * [Third party](#third-party)
      
      
# WebRTC流媒体网关服务器
用于将H5通过WebRTC上传的媒体流(STUN/TURN/SRTP)转接到其他媒体流服务器(RTP/RTMP/HTTP-FLV/HLS)

## How it works
- 接收客户端SDP信令(Websocket)
- 接收WebRTC客户端推送音视频流(SRTP/SRTCP), 并转发到Nginx(RTMP), 也可以自定义转发脚本
- 视频只支持接收H264, 不是Baseline profile的会进行转码, 音频只支持Opus, 转码AAC
- 支持外部HTTP接口[登录校验/上下线通知/在线列表(Websocket)同步]

## Compile and Installation
### Compile
```bash
cd $your_WebRTC-Server_path/Server/dep && ./extract.sh.sh
cd $your_WebRTC-Server_path/Server && ./compile.sh
```
### Getting started on Linux
Build tar package
```bash
cd $your_WebRTC-Server_path/Server && ./package.sh
```
Install tar package to /app/live/mediaserver
```bash
cd $your_WebRTC-Server_path/Server/package && tar zxvf local-$version.tar.gz && cd local && ./install.sh
```
### Getting started on Docker
Build docker image
```bash
cd $your_WebRTC-Server_path/Server/docker && ./mediaserver_build.sh
```
Now you can run on local machine
```bash
cd $your_WebRTC-Server_path/Server/docker && ./mediaserver_run.sh
```
Or push it to your docker server
```bash
cd $your_WebRTC-Server_path/Server/docker
sed -i 's/^HOST=.*/HOST=$your_docker_server/g' ./mediaserver_push.sh
./mediaserver_push.sh
```
### Getting started on Kubernetes
For Kubernetes v1.7+
```bash
cd $your_WebRTC-Server_path/Server/docker
kubectl apply -f mediaserver-deploy.yaml
```

## 运行界面
![](https://github.com/KingsleyYau/WebRTC-Server/blob/master/demo.png?raw=true)

## Test Page
https://github.com/KingsleyYau/WebRTC-Server/blob/master/Server/sig/src/static/index.html

## Sequence Diagram
![](https://github.com/KingsleyYau/WebRTC-Server/blob/master/Server/doc/MediaServer_Call_Sequence.png?raw=true)

## Third Party
[coturn](https://github.com/coturn/coturn)</br>
[ffmpeg](https://www.ffmpeg.org/)</br>
[libev](http://software.schmorp.de/pkg/libev.html)</br>
[openssl](https://www.openssl.org/)</br>
[curl](https://curl.haxx.se/)</br>
[nice](https://github.com/libnice/libnice)</br>
[srtp](https://github.com/cisco/libsrtp)</br>
[websocket](https://github.com/zaphoyd/websocketpp)</br>
