Catalog
=================

   * [WebRTC流媒体网关服务器](#WebRTC流媒体网关服务器)
   		* [功能](#功能)
      * [第三方开源](#第三方开源)
      
      
# 流媒体推拉流模块
## 功能
- 接收客户端SDP信令(Websocket)
- 接收WebRTC客户端推送音视频流(SRTP/SRTCP), 并转发到Nginx(RTMP)

### 时序图
![](https://github.com/KingsleyYau/WebRTC-Server/blob/master/Server/doc/MediaServer_Call_Sequence.png?raw=true)

## 第三方开源
[nice](https://github.com/libnice/libnice)</br>
[srtp](https://github.com/cisco/libsrtp)</br>
[websocket](https://github.com/zaphoyd/websocketpp)</br>