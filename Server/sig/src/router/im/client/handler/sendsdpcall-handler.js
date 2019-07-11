/*
* 发送SDP逻辑处理类
* Author: Max.Chiu
* */

// 项目公共库
const Common = require('../../../../lib/common');
const Http = require('http');
const QueryString = require('querystring');

// 在线用户
const OnlineUserManager = require('../../../../user/online-users').OnlineUserManager;
// 业务管理器
const BaseHandler = require('./base-handler');
// 推送消息
const NoticeSender = require('../notice-sender/notice-sender');
const SendSdpCallNotice = require('../notice/sendsdpcall-notice');

module.exports = class SendSdpCallHandler extends BaseHandler {
    constructor() {
        super();
    }

    static getRoute() {
        return 'imShare/sendSdpCall';
    }

    async handle(ctx, reqData) {
        return await new Promise( async (resolve, reject) => {
            let bFlag = false;
            let user = this.getBaseRespond(ctx, reqData);

            if( !Common.isNull(user) ) {
                // 查找目标用户
                if( !Common.isNull(reqData.req_data.toUserId) ) {
                    OnlineUserManager.getInstance().getUserWithId(reqData.req_data.toUserId).then( async (userList) => {
                        for (let i = 0; i < userList.length; i++) {
                            let desUser = userList[i];

                            // 发送消息到用户
                            let sender = new NoticeSender();
                            let notice = new SendSdpCallNotice(user.userId, desUser.userId, reqData.req_data.sdp);
                            sender.send(desUser.userId, notice);
                        }
                    });
                }

                // var data = QueryString.stringify({
                //     'sdp': reqData.req_data.sdp,
                // });

                var obj = {
                    'sdp' : reqData.req_data.sdp,
                }
                var data = JSON.stringify(obj);

                var options = {
                    hostname: '192.168.88.133',
                    port: 9880,
                    path: '/CALLSDP',
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                        'Content-Length': data.length
                    }
                }

                var req = Http.request(options, res => {
                    Common.log('im', 'info', '[' + ctx.socketId + ']-CallSdp, ' + ' status:' + res.statusCode + ', header:' + JSON.stringify(res.headers));

                    res.setEncoding('utf8');
                    res.on('data', function (data) {
                        Common.log('im', 'info', '[' + ctx.socketId + ']-CallSdp, ' + ' body:' + data);
                    });
                    res.on('error', function (err) {
                        Common.log('im', 'info', '[' + ctx.socketId + ']-CallSdp, ' + ' err:' + err);
                    });
                });

                req.on('error', function (err) {
                    Common.log('im', 'info', '[' + ctx.socketId + ']-CallSdp, ' + ' err:' + err);
                });
                req.write(data);
                req.end();
            }

            resolve(this.respond);
        });
    }
}

