// 异步框架
const Koa = require('koa');
// Websocket
const websockify = require('koa-websocket');
// Websocket
const wss = require('koa-wss');
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
// Redis
// const redisClient = require('./lib/redis-connector').RedisConnector.getInstance();
// Model的Keys
const DBModelKeys = require('./db/model-keys');
// 用户
const OnlineUserManager = require('./user/online-users').OnlineUserManager;
// 项目接口
const mainRouter = require('./router/im/client/handler/client-main-router');
const interMainRouter = require('./router/im/server/handler/server-main-router');

// module.exports = class ImService {
class ImService {
    constructor() {
        // 创建内部服务
        this.createInternalServer();

        // 创建外部服务
        this.createExternalServer();
    }

    createExternalServer() {
        // SSL options
        const options = {
            key: Fs.readFileSync('./etc/server.key'),  // ssl文件路径
            cert: Fs.readFileSync('./etc/server.crt')  // ssl文件路径
        };

        // 创建异步框架
        // WSS
        let koa_wss = new Koa();
        this.exApps = wss(koa_wss, {}, options);

        // WS
        let koa = new Koa();
        this.exApp = websockify(koa);

        // 增加公共处理
        this.exApps.ws.use( async (ctx, next) => {
            // return `next` to pass the context (ctx) on to the next ws middleware

            // 新的连接, 生成SocketId
            ctx.socketId = 'SOCKETID-' + Math.random().toString(36).substr(2).toLocaleUpperCase();
            // 记录连接时间
            let curTime = new Date();
            ctx.connectTtime = curTime.getTime();

            Common.log('im', 'debug', '[' + ctx.socketId + ']-Im Client Connected');

            // 等待其他中间件处理的异步返回
            await next();
            // 所有中间件处理完成

            // 增加在线数量
            let appInfo = Common.appInfo();
            // redisClient.client.incr(appInfo.serverUniquePattern,
            //     async (err, res) => {
            //     Common.log('im', 'debug', '[' + ctx.socketId  + ']-Im Client ServerOnlineCountKey, ' + res + ', err:' + err);
            // });
        });
        // 增加路由
        this.exApps.ws.use(mainRouter.routes());

        /***************************************************************/
        // 增加公共处理
        this.exApp.ws.use( async (ctx, next) => {
            // return `next` to pass the context (ctx) on to the next ws middleware

            // 新的连接, 生成SocketId
            ctx.socketId = 'SOCKETID-' + Math.random().toString(36).substr(2).toLocaleUpperCase();
            // 记录连接时间
            let curTime = new Date();
            ctx.connectTtime = curTime.getTime();

            Common.log('im', 'debug', '[' + ctx.socketId + ']-Im Client Connected');

            // 等待其他中间件处理的异步返回
            await next();
            // 所有中间件处理完成

            // 增加在线数量
            let appInfo = Common.appInfo();
            // redisClient.client.incr(appInfo.serverUniquePattern,
            //     async (err, res) => {
            //         Common.log('im', 'debug', '[' + ctx.socketId  + ']-Im Client ServerOnlineCountKey, ' + res + ', err:' + err);
            //     });
        });
        // 增加路由
        this.exApp.ws.use(mainRouter.routes());
    }

    createInternalServer() {
        // 创建异步框架
        let koa = new Koa();
        this.inApp = Http.createServer(koa.callback());
        this.io = new SocketIO(this.inApp, {
            parser: jsonParser
        });

        // socket连接
        this.io.on('connection', (socket) => {
            // 记录连接时间
            let curTime = new Date();
            socket.connectTtime = curTime.getTime();
            Common.log('im-server', 'debug', '[' + socket.id + ']-Im Server Connected');

            interMainRouter(socket);
        });
    }

    start(opts) {
        // 启动服务器
        opts = opts || {};

        let exPort = AppConfig.exApp.port;
        // if( !Common.isNull(opts.number) ) {
        //     exPort += parseInt(opts.number);
        // }
        AppConfig.exApp.port = exPort;
        this.exApp.listen(exPort);
        this.exApps.listen(AppConfig.exApps.port);

        let inPort = AppConfig.inApp.port;
        if( !Common.isNull(opts.number) ) {
            inPort += parseInt(opts.number);
        }
        AppConfig.inApp.port = inPort;
        this.inApp.listen(inPort);

        Common.log('im', 'fatal', 'Im service start in exApps : ' + AppConfig.exApps.port + ', exApp : ' + AppConfig.exApp.port + ', inPort : ' + inPort);
    }
}

function handle(signal) {
    Common.log('main', 'fatal', 'Im exit pid : ' + process.pid + ', env : ' + process.env.NODE_ENV);
    // 清空缓存用户列表
    OnlineUserManager.getInstance().logoutAllLocalUsers();
    // 退出程序
    process.exit();
}

process.on('SIGINT', handle);
process.on('SIGTERM', handle);
process.on('SIGTERM', handle);

let number;
if( process.argv.length > 2 ) {
    number = parseInt(process.argv[2], 10);
}

// 启动Im
im = new ImService();
im.start({number:number});