/*
* 断开用户连接消息类
* Author: Max.Chiu
* */

// 项目公共库
const Common = require('../../../../lib/common');
// 业务管理器
const BaseNotice = require('./base-notice');

module.exports = class KickNotice extends BaseNotice {
    constructor(userId) {
        super();
        this.obj.noticeData.req_data.userId = userId;
        this.obj.isKick = true;
    }

    static getRoute() {
        return 'imShare/kickNotice';
    }

    async send(user) {
        return super.send(user);
    }
}