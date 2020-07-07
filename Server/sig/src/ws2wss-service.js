// 异步框架
const Koa = require('koa');
// Websocket
const ws = require('koa-websocket');
// Websocket
const wss = require('koa-wss');
const WebSocketClient = require('ws');
// Http框架
const Http = require('http');
// 公共库
const Fs = require('fs');
const Path = require('path');
// SocketIO
const SocketIO = require('socket.io');
const jsonParser = require('socket.io-json-parser');
// 项目公共库
const Common = require('./lib/common');
const AppConfig = require('./config/app-config');

// module.exports = class ProxyService {
class ProxyService {
    constructor() {
        // 创建外部服务
        this.createExternalServer();
        this.proxyHost = AppConfig.proxy.proxyHost;
    }

    createExternalServer(code, data) {
        // SSL options
        const options = {
            // key: Fs.readFileSync('./etc/server.key'),  // ssl文件路径
            // cert: Fs.readFileSync('./etc/server.crt')  // ssl文件路径
        };

        // 创建异步框架
        // WSS
        let koa_wss = new Koa();
        // this.app = ws(koa_wss, {}, options);
        this.app = ws(koa_wss, {});
        this.app.ws.use( async (ctx, next) => {
            // return `next` to pass the context (ctx) on to the next ws middleware
            // 新的连接, 生成SocketId
            ctx.socketId = 'SOCKETID-' + Math.random().toString(36).substr(2).toLocaleUpperCase();
            Common.log('ws2wss', 'info', '[' + ctx.socketId + ']-Client.connected');

            try {
                var userAgent = ctx.req.headers["user-agent"];
                // var proxy = new WebSocketClient(this.proxyHost);
                var proxyHeaders = {headers:{"user-agent":userAgent}};
                var proxy = new WebSocketClient(this.proxyHost + ctx.originalUrl, "", proxyHeaders);
                proxy.on('message', (message) => {
                    Common.log('ws2wss', 'info', '[' + ctx.socketId + ']-Proxy.message.length, ' + message.length);
                    if ( ctx.websocket.readyState == ctx.websocket.OPEN ) {
                        ctx.websocket.send(message);
                    }
                });
                proxy.on('close', () => {
                    Common.log('ws2wss', 'info', '[' + ctx.socketId + ']-Proxy.close, [代理断开]');
                    ctx.websocket.close();
                });
                proxy.on('error', (err) => {
                    Common.log('ws2wss', 'info', '[' + ctx.socketId + ']-Proxy.error, [代理出错], ' + err);
                    ctx.websocket.close();
                });

                ctx.websocket.on('close', function (err) {
                    Common.log('ws2wss', 'info', '[' + ctx.socketId + ']-Client.close, [客户端断开], ' + err);
                    try {
                        proxy.close(1000, err.toString());
                    } catch (e) {
                        Common.log('ws2wss', 'info', '[' + ctx.socketId + ']-Client.close, [客户端断开], e : ' + e.toString());
                    }
                });
                ctx.websocket.on('error', function (err) {
                    Common.log('ws2wss', 'error', '[' + ctx.socketId + ']-Client.error, [客户端断开], ' + err);
                    try {
                        proxy.close(1000, err.toString());
                    } catch (e) {
                        Common.log('ws2wss', 'error', '[' + ctx.socketId + ']-Client.error, [客户端断开], e : ' + e.toString());
                    }
                });

                ctx.websocket.on('message', async (message) => {
                    Common.log('ws2wss', 'info', '[' + ctx.socketId + ']-Client.message.length, ' + message.length);
                    if (proxy.readyState == proxy.OPEN) {
                        Common.log('ws2wss', 'info', '[' + ctx.socketId + ']-Proxy.send, [' + ctx.socketId + ']->[' + this.proxyHost + '], message.length:' + message.length);
                        proxy.send(message);
                    } else {
                        Common.log('ws2wss', 'info', '[' + ctx.socketId + ']-Proxy, [等待代理连接], [' + ctx.socketId + ']->[' + this.proxyHost + ']');
                        await new Promise(function (resolve) {
                            proxy.on('open', () => {
                                Common.log('ws2wss', 'info', '[' + ctx.socketId + ']-Proxy.send, [' + ctx.socketId + ']->[' + this.proxyHost + '], message.length:' + message.length);
                                proxy.send(message);
                                resolve();
                            });
                        }.bind(this));
                    }
                });
            } catch (e) {
                Common.log('ws2wss', 'error', '[' + ctx.socketId + ']-Proxy.connect.error, [代理连接失败], e : ' + e.toString());
                ctx.websocket.close();
            }

            // 等待其他中间件处理的异步返回
            await next();
            // 所有中间件处理完成
        });
    }

    start(opts) {
        // 启动服务器
        opts = opts || {};

        let port = AppConfig.proxy.port;
        if( !Common.isNull(opts.number) ) {
            port = parseInt(opts.number);
        }
        this.app.listen(port);

        if( !Common.isNull(opts.host) ) {
            this.proxyHost = opts.host;
        }


        Common.log('ws2wss', 'fatal', 'ws2wss service start in port : ' + port + ', host : ' + this.proxyHost);
        for (let i = 0; i < process.argv.length; i++) {
            Common.log('ws2wss', 'fatal', 'ws2wss service param[' + i + ']:' + process.argv[i]);
        }
    }
}

function handle(signal) {
    Common.log('main', 'fatal', 'ws2wss service exit pid : ' + process.pid + ', env : ' + process.env.NODE_ENV);
    // 退出程序
    process.exit();
}

process.on('SIGINT', handle);
process.on('SIGTERM', handle);
process.on('SIGTERM', handle);

process.on('uncaughtException', function(err) {
    Common.log('main', 'fatal', 'ws2wss service exit pid : ' + process.pid + ', stack : ' + err.stack);
});

let number;
if( process.argv.length > 2 ) {
    number = parseInt(process.argv[2], 10);
}

process.env.NODE_TLS_REJECT_UNAUTHORIZED = "0"
// 启动
// JP
proxy = new ProxyService();
proxy.start({number:9084, host:"wss://52.196.96.7:9082"});
// // EU
// proxy2 = new ProxyService();
// proxy2.start({number:9085, host:"wss://18.194.23.38:9082"});