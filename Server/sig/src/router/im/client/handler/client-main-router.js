/*
* Im主路由
* */

// 路由
const Router = require('koa-router');
// 项目公共库
const Common = require('../../../../lib/common');
// Redis
// const redisClient = require('../../../../lib/redis-connector').RedisConnector.getInstance();
// Model的Keys
const DBModelKeys = require('../../../../db/model-keys');
// 用户
const User = require('../../../../user/users').User;
const OnlineUserManager = require('../../../../user/online-users').OnlineUserManager;
// 业务逻辑处理
const HandleRouter = require('./client-bridge-router').HandleRouter;
const HeartBeatHandler = require('./heartbeat-handler');

// 设置路由
let clientMainRouter = new Router();

function disconnect(ctx) {
    try {
        // 增加在线数量
        let appInfo = Common.appInfo();
        // redisClient.client.DECR(appInfo.serverUniquePattern, async (err, res) => {
        //     Common.log('im', 'debug', '[' + ctx.socketId  + ']-ClientMainRouter.disconnect, ' + res + ', err:' + err);
        // });

        let user = OnlineUserManager.getInstance().getUserWithSocket(ctx.socketId);
        if( !Common.isNull(user) ) {
            OnlineUserManager.getInstance().logout(user);
        }
    } catch (e) {
        Common.log('im', 'error', '[' + ctx.socketId + ']-ClientMainRouter.disconnect, [' + ctx.userId + '], ' + e);
    }
}

clientMainRouter.all('/', async (ctx, next) => {
    // 等待异步接口
    await new Promise(function (resolve, reject) {
        ctx.websocket.on('message', async (message) => {
            let reqData = {};

            try {
                reqData = JSON.parse(message);
                // 过滤心跳日志
                if (reqData.route != HeartBeatHandler.getRoute()) {
                    Common.log('im', 'info', '[' + ctx.socketId + ']-ClientMainRouter.request, [' + ctx.userId + '], ' + message);
                }
            }
            catch (e) {
                Common.log('im', 'error', '[' + ctx.socketId + ']-ClientMainRouter.request, [' + ctx.userId + '], ' + message + ', ' + e );
            }

            let data = '';
            let handlerRespond = {};
            let handler = null;

            // 路由分发
            let handlerCls = HandleRouter.getInstance().getRouterByName(reqData.route);
            if( handlerCls ) {
                handler = new handlerCls();
            }

            if( handler ) {
                // 统一处理返回
                await handler.handle(ctx, reqData).then( (respond) => {
                    handlerRespond = respond;
                }).catch( (err) => {
                    Common.log('im', 'info', '[' + ctx.socketId + ']-ClientMainRouter.handle, err: ', err.message + ', stack: ' + err.stack);
                });

                if( !Common.isNull(handlerRespond.resData) && handlerRespond.resData != '' ) {
                    // 需要返回的命令
                    let json = '';

                    try {
                        json = JSON.stringify(handlerRespond.resData);
                        ctx.websocket.send(json);

                        // 过滤心跳日志
                        if (reqData.route != HeartBeatHandler.getRoute()) {
                            Common.log('im', 'info', '[' + ctx.socketId + ']-ClientMainRouter.respond, [' + ctx.userId + '], ' + json);
                        }
                    } catch (e) {
                        Common.log('im', 'error', '[' + ctx.socketId + ']-ClientMainRouter.respond, [' + ctx.userId + '], ' + json + ', ' + e);
                        ctx.websocket.close();
                    }
                }

                if(handlerRespond.isKick) {
                    // 需要断开客户端
                    ctx.websocket.close();
                }

                resolve(handlerRespond);
            } else {
                ctx.websocket.close();
                reject();
            }

        })

        ctx.websocket.on('close', function (err) {
            Common.log('im', 'info', '[' + ctx.socketId + ']-ClientMainRouter.close, [客户端断开], ' + err);

            disconnect(ctx);
            reject(err);
        });

        ctx.websocket.on('error', function (err) {
            Common.log('im', 'error', '[' + ctx.socketId + ']-ClientMainRouter.error, [客户端断开], ' + err);

            disconnect(ctx);
            reject(err);
        });

    }.bind(this)).then().catch(
        (err) => {
            Common.log('im', 'error', '[' + ctx.socketId + ']-ClientMainRouter.catch, [客户端断开], ' + err);

            disconnect(ctx);
        }
    );
});

module.exports = clientMainRouter;