/*
* 发送SDP逻辑处理类
* Author: Max.Chiu
* */

// 项目公共库
const Common = require('../../../../lib/common');
const Http = require('http');
const QueryString = require('querystring');
const AppConfig = require('../../../../config/app-config');

// 在线用户
const OnlineUserManager = require('../../../../user/online-users').OnlineUserManager;
// 业务管理器
const BaseHandler = require('./base-handler');

module.exports = class SendGetTokenHandler extends BaseHandler {
    constructor() {
        super();
    }

    static getRoute() {
        return 'imRTC/sendGetToken';
    }

    async handle(ctx, reqData) {
        return await new Promise( async (resolve, reject) => {
            let bFlag = false;
            let user = this.getBaseRespond(ctx, reqData);

            if( !Common.isNull(user) ) {
                if( !Common.isNull(reqData.req_data.user_id) ) {
                    let iceServers = [];
                    let urls = AppConfig.ice;

                    let curTime = new Date();
                    let expire = curTime.getTime() + 600;

                    let iceuser = expire + ':' + reqData.req_data.user_id;
                    let sha1 = Common.sha1(iceuser, 'mediaserver12345');
                    let base64 = sha1.toString('base64');
                    let server = {
                        urls:urls,
                        username:iceuser,
                        credential:base64,
                    }
                    iceServers.push(server);
                    this.respond.resData.data = {
                        iceServers:iceServers,
                    }
                } else {
                    this.respond.resData.errno = 10003;
                    this.respond.resData.errmsg = 'Parameter "user_id" Is Missing.';
                }
            } else {
                this.respond.resData.errno = 10002;
                this.respond.resData.errmsg = 'Not Login Yet.';
            }

            resolve(this.respond);
        });
    }
}

