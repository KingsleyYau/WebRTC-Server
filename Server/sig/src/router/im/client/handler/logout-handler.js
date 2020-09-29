/*
* 断开客户端逻辑处理类
* Author: Max.Chiu
* */

// 项目公共库
const Common = require('../../../../lib/common');
// 在线用户
const OnlineUserManager = require('../../../../user/online-users').OnlineUserManager;
// 业务管理器
const BaseHandler = require('./base-handler');

module.exports = class LogoutHandler extends BaseHandler {
    constructor() {
        super();
    }

    static getRoute() {
        return 'imP2P/sendLogout';
    }

    async handle(ctx, reqData) {
        return new Promise(function (resolve, reject) {
            let user = this.getBaseRespond(ctx, reqData);
            if( !Common.isNull(user)  ) {
                this.respond.isKick = true;
            }

            resolve(this.respond);
        }.bind(this));
    }
}

