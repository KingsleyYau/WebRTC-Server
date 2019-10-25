Catalog
=================

   * [WebRTC流媒体网关服务器](#WebRTC流媒体网关服务器)
   		* [功能](#功能)
      * [第三方开源](#第三方开源)
      
      
# WebRTC流媒体网关服务器
## 功能
- 接收客户端SDP信令(Websocket)
- 接收WebRTC客户端推送音视频流(SRTP/SRTCP), 并转发到Nginx(RTMP), 也可以自定义转发脚本
- 视频只支持接收H264, 不是Baseline profile的会进行转码, 音频只支持Opus, 转码AAC
- 支持外部HTTP接口[登录校验/上下线通知/在线列表(Websocket)同步]

### 时序图
![](https://github.com/KingsleyYau/WebRTC-Server/blob/master/Server/doc/MediaServer_Call_Sequence.png?raw=true)

### 运行界面
![](https://github.com/KingsleyYau/WebRTC-Server/blob/master/demo.png?raw=true)

### 测试页面
https://github.com/KingsleyYau/WebRTC-Server/blob/master/Server/sig/src/static/index.html

## 第三方开源
[coturn](https://github.com/coturn/coturn)</br>
[ffmpeg](https://www.ffmpeg.org/)</br>
[libev](http://software.schmorp.de/pkg/libev.html)</br>
[openssl](https://www.openssl.org/)</br>
[curl](https://curl.haxx.se/)</br>
[nice](https://github.com/libnice/libnice)</br>
[srtp](https://github.com/cisco/libsrtp)</br>
[websocket](https://github.com/zaphoyd/websocketpp)</br>