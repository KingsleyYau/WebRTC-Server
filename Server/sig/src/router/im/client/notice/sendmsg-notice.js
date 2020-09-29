/*
* 推送文本消息类
* Author: Max.Chiu
* */

// 项目公共库
const Common = require('../../../../lib/common');
// 在线用户
const OnlineUserManager = require('../../../../user/online-users').OnlineUserManager;
// 业务管理器
const BaseNotice = require('./base-notice');

module.exports = class SendMsgNotice extends BaseNotice {
    constructor(fromUserId, toUserId, msg) {
        super();

        this.obj.noticeData.req_data.userId = fromUserId;
        this.obj.noticeData.req_data.msg = '[' + fromUserId + ']: ' + msg;
    }

    static getRoute() {
        return 'imP2P/sendChatNotice';
    }
}