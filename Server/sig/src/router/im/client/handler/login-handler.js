/*
* 登录逻辑处理类
* Author: Max.Chiu
* */

// 项目公共库
const Common = require('../../../../lib/common');

// 用户
const User = require('../../../../user/users').User;
const OnlineUserManager = require('../../../../user/online-users').OnlineUserManager;
// 业务管理器
const BaseHandler = require('./base-handler');
// 消息推送类
const NoticeSender = require('../notice-sender/notice-sender');
const KickNotice = require('../notice/kick-notice');

module.exports = class LoginHandler extends BaseHandler {
    constructor() {
        super();

        this.route = LoginHandler.getRoute();
    }

    static getRoute() {
        return 'imP2P/sendLogin';
    }

    async handle(ctx, reqData) {
        return new Promise(async (resolve, reject) => {
            let user = null;
            if( !Common.isNull(reqData.req_data.userId) ) {
                // 如果已经登录, 直接返回
                let oldUser = OnlineUserManager.getInstance().getUserWithSocket(ctx.socketId);
                if( Common.isNull(oldUser) ) {
                    // 创建新用户
                    let curTime = new Date();
                    user = User.createUserWithLogin(reqData.req_data.userId, ctx.socketId, ctx.websocket, ctx.connectTtime, curTime.getTime());

                    // 如果用户已经登录, 通知用户被踢下线
                    let sender = new NoticeSender();
                    let notice = new KickNotice(user.userId);
                    await sender.send(user.userId, notice);

                    // 踢出旧用户
                    await OnlineUserManager.getInstance().getUserWithId(user.userId).then( async (userList) => {
                        // 删除旧用户信息
                        for (let i = 0; i < userList.length; i++) {
                            let oldUser = userList[i];
                            if (!Common.isNull(oldUser.websocket)) {
                                Common.log('im', 'warn', '[' + user.userId + ']-LoginHandler.handle, [踢出旧用户], ' +
                                    '[' + oldUser.socketId + '], '+
                                    '[' + oldUser.serverHost + ':' + oldUser.serverPort + ']');

                                oldUser.websocket.close();
                            }
                        }
                    }).catch( () => {

                    });

                    // 等待登录处理
                    await OnlineUserManager.getInstance().login(user);
                } else {
                    // 已经登录的用户, 直接返回登录信息
                }
            }

            user = this.getBaseRespond(ctx, reqData);
            if( !Common.isNull(user) ) {
                // 登录成功
                ctx.userId = user.userId;

                this.respond.resData.data = {
                    socketId:user.socketId,
                    userId:user.userId,
                }
            } else {
                // 登录失败
                this.respond.resData.errno = 10002;
                this.respond.resData.errmsg = 'login fail.';
            }

            resolve(this.respond);
        });
    }
}

