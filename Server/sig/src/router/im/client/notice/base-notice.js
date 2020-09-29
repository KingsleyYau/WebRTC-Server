/*
* 推送消息基类
* Author: Max.Chiu
* */

// 项目公共库
const Common = require('../../../../lib/common');
// Redis
// const redisClient = require('../../../../lib/redis-connector').RedisConnector.getInstance();
// 用户
const Users = require('../../../../user/users');
// 在线用户
const OnlineUserManager = require('../../../../user/online-users').OnlineUserManager;

module.exports = class BaseNotice {
    constructor() {
        this.obj = {
            noticeData:{
                id:0,
                route:'',
                errno:0,
                errmsg:'',
                req_data:{

                }
            },
            isKick:false
        }

        // 生成路由
        this.obj.noticeData.route = this.constructor.getRoute();
    }

    static getRoute() {
        return 'im/BaseRoute';
    }

    async send(user) {
        // 生成数据
        let url = user.serverHost + ':' + user.serverPort;
        let json = JSON.stringify(this.obj.noticeData);
        Common.log('im', 'info', '[' + user.userId + ']-BaseNotice.send, [' + url + '], ' + json);
        try {
            user.websocket.send(json);
        } catch (err) {
            Common.log('im', 'error', '[' + user.socketId + ']-BaseNotice.send, [' + url + '], ' + 'error: ' + err.message);

            // 删除本地过期用户
            OnlineUserManager.getInstance().logout(user);
        }
    }
}