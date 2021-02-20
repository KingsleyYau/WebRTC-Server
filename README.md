Catalog
=================

   * [WebRTC流媒体网关服务器](#WebRTC流媒体网关服务器)
   		* [How it works](#how-it-works)
   		* [Compile & Installation](#compile-and-installation)
   		* [Sequence diagram](#sequence-diagram)
   		* [Test page](#test-page)
   		* [Example Config](#example-config)
      * [Third party](#third-party)
      
      
# WebRTC流媒体网关服务器
用于将H5通过WebRTC上传的媒体流(STUN/TURN/SRTP)转接到其他媒体流服务器(RTP/RTMP/HTTP-FLV/HLS)

## How it works
- 接收客户端SDP信令(Websocket)
- 接收WebRTC客户端推送音视频流(SRTP/SRTCP), 并转发到Nginx(RTMP), 也可以自定义转发脚本
- 视频只支持接收H264, 不是Baseline profile的会进行转码, 音频只支持Opus, 转码AAC
- 支持外部HTTP接口[登录校验/上下线通知/在线列表(Websocket)同步]
- 支持接收端动态码率算法(Receive side gcc)
- RTT达到200ms的情况下, 可以支持Full HD(1920x1080), 峰值3kbps, 均值2kbps

### Sequence Diagram
![](https://github.com/KingsleyYau/WebRTC-Server/blob/master/Server/doc/MediaServer_Call_Sequence.png?raw=true)

## Compile and Installation
### Compile
```bash
cd $your_WebRTC-Server_path/Server/dep && ./extract.sh
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
cd $your_WebRTC-Server_path/Server/docker && mediaserver_build.sh
```
Now you can run on local machine
```bash
cd $your_WebRTC-Server_path/Server/docker && mediaserver_run.sh
```
Or push it to your docker server
```bash
cd $your_WebRTC-Server_path/Server/docker
sed -i 's/^HOST=.*/HOST=$your_docker_server/g' mediaserver_push.sh
./mediaserver_push.sh
```
### Getting started on Kubernetes
For Kubernetes v1.7+
```bash
cd $your_WebRTC-Server_path/Server/docker
kubectl apply -f mediaserver-deploy.yaml
```
Deploy a service for mediaserver to communicate with nginx service. For example, I have already deployed a nginx outside k8s
```bash
cd $your_WebRTC-Server_path/Server/docker
sed -i 's/^- ip: .*/- ip: $your_nginx_server/g' rtmp-svc.yaml
kubectl apply -f rtmp-svc.yaml
```

### Example config
#### mediaserver.config
```
# MediaServer Config File
# 基本配置
[BASE]
# HTTP端口
PORT=9880
# 最大连接数
MAXCLIENT=100
# 处理线程数目, 一般为cpu内核数目
MAXHANDLETHREAD=1
# 单核每秒处理请求数目
MAXQUERYPERCOPY=0
# 单个HTTP请求超时时间(秒), 0为不超时
TIMEOUT=0
# 统计信息输出时间间隔(秒)
STATETIME=1800
# 进程PID文件路径
PID=run/mediaserver.pid

# 日志配置
[LOG]
# 日志级别(0-6, 0:关闭 1:系统错误 2:程序错误 3:警告 4:业务流程 5:详细信息 6:火力全开(所有数据包和变量))
LOGLEVEL=4
# 日志路径
LOGDIR=log/mediaserver
# 调试模式(0:关闭, 1:开启)
DEBUGMODE=0

# WebRTC配置
[WEBRTC]
# 媒体流转发起始端口
WEBRTCPORTSTART=10000
# 最大媒体流转发数
WEBRTCMAXCLIENT=100
# 执行转发RTMP的脚本
RTP2RTMPSHELL=/app/live/mediaserver/script/rtp2rtmp.sh
# 执行转发RTMP的地址
RTP2RTMPBASEURL=rtmp://127.0.0.1/live
# 执行转发RTMP的地址(全录制)
RTP2RTMPBASERECORDURL=rtmp://127.0.0.1/live_allframe
# 执行转发RTP的脚本
RTMP2RTPSHELL=/app/live/mediaserver/script/rtmp2rtp.sh
# 执行转发RTP的地址
RTMP2RTPBASEURL=rtmp://127.0.0.1/live_play
# 证书路径
DTLSCER=/app/live/mediaserver/etc/webrtc_dtls.crt
DTLSKEY=/app/live/mediaserver/etc/webrtc_dtls.key

# 需要绑定的本地IP(默认为空)
ICELOCALIP=192.168.88.133
# STUN服务器的内网IP(默认为127.0.0.1)
#STUNSERVERIP=192.168.88.133
# STUN服务器外网IP(用于返回给客户端, 没有则默认和STUNSERVERIP一样)
STUNSERVEREXTIP=192.168.88.133
# [用户名/密码]和[共享密钥]只能2选1, 0表示使用[用户名/密码], 1表示使用[共享密钥]
TURNSTATIC=1
# TURN用户名
TURNUSERNAME=Username
# TURN密码
TURNPASSWORD=123
# TURN共享密钥
TURNSHARESECRET=usersecrt123456
# TURN共享密钥客户端有效时间(秒)
TURNCLIENTTTL=3600

# RTP配置
[RTP]
# 最大PLI间隔(秒)
RTP_PLI_MAX_INTERVAL=3
# 是否开启自适应码率控制(Receive side gcc)
RTP_AUTO_BITRATE=1
# 最小视频码率(bps)
RTP_MIN_VIDEO_BPS=200000
# 最大视频码率(bps)
RTP_MAX_VIDEO_BPS=1000000
# 是否开启模拟丢包
RTP_TEST=0
```

## Running in interactive model
![](https://github.com/KingsleyYau/WebRTC-Server/blob/master/demo.png?raw=true)

## Test Page
https://github.com/KingsleyYau/WebRTC-Server/blob/master/Server/sig/src/static/index.html

## Third Party
[coturn](https://github.com/coturn/coturn)</br>
[ffmpeg](https://www.ffmpeg.org/)</br>
[libev](http://software.schmorp.de/pkg/libev.html)</br>
[openssl](https://www.openssl.org/)</br>
[curl](https://curl.haxx.se/)</br>
[nice](https://github.com/libnice/libnice)</br>
[srtp](https://github.com/cisco/libsrtp)</br>
[websocket](https://github.com/zaphoyd/websocketpp)</br>

### Third Party Modification
#### libnice
- 修复timer使用不当导致crash
- 修复内存泄漏(主要为g_socket_source回调中销毁该source, timer没有对应unref, 被外部获取candidate列表后没有对应释放函数等)
- 增加TCP的epoll支持

#### websocketapp
- 修复内存泄漏
- 开放devel日志

#### libsdp
- 修改当sdp中其中一个candidate解析失败时候则跳过, 并继续下一个(原来为直接返回解析失败)