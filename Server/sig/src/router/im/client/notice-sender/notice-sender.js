/*
* 推送消息基类
* Author: Max.Chiu
* */

// Socket.IO
const io = require('socket.io-client');
const jsonParser = require('socket.io-json-parser');

// 项目公共库
const Common = require('../../../../lib/common');
// 用户
const Users = require('../../../../user/users');
// 在线用户
const OnlineUserManager = require('../../../../user/online-users').OnlineUserManager;

module.exports = class NoticeSender {
    constructor() {
    }

    async send(userId, notice) {
        let manager = OnlineUserManager.getInstance();
        return OnlineUserManager.getInstance().getUserWithId(userId).then( (userList) => {
            for(let i = 0; i < userList.length; i++) {
                let appInfo = Common.appInfo();
                let user = userList[i];
                let url = user.serverHost + ':' + user.serverPort;

                if( user.serverHost == appInfo.serverHost && user.serverPort == appInfo.serverPort ) {
                    // 增加本地推送Id
                    notice.obj.noticeData.id = user.noticeId++;

                    // 本地用户, 直接发送
                    let json = JSON.stringify(notice.obj.noticeData);
                    Common.log('im', 'debug', '[' + userId + ']-NoticeSender.send, Local User, [' + url + '], ' + json);
                    notice.send(user);

                } else {
                    // 外部服务, 发送到对应服务
                    let json = JSON.stringify(notice.obj.noticeData.req_data);
                    Common.log('im', 'debug', '[' + userId + ']-NoticeSender.send, Remote User, [' + url + '], ' + json);
                    let client = io(url, {parser:jsonParser});
                    try {
                        client.emit(notice.obj.noticeData.route, {
                            id: 0,
                            route:notice.obj.noticeData.route,
                            req_data: notice.obj.noticeData.req_data,
                            socketId:user.socketId,
                            isKick:notice.obj.isKick
                        });
                    } catch (err) {
                        Common.log('im', 'error', '[' + user.socketId + ']-NoticeSender.send, Remote User, [' + url + '], ' + 'error: ' + err.message);
                    }
                }
            }
        }).catch( () => {

        });
    }

}