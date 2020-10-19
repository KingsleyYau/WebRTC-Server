/*
* 心跳逻辑处理类
* Author: Max.Chiu
* */

// 项目公共库
const Common = require('../../../../lib/common');
// 在线用户
const OnlineUserManager = require('../../../../user/online-users').OnlineUserManager;
// 业务管理器
const BaseHandler = require('./base-handler');

module.exports = class HeartBeatHandler extends BaseHandler {
    constructor() {
        super();
    }

    static getRoute() {
        return 'imRTC/sendPing';
    }

    async handle(ctx, reqData) {
        return new Promise( async (resolve, reject) => {
            let user = this.getBaseRespond(ctx, reqData);
            resolve(this.respond);
        });
    }
}

