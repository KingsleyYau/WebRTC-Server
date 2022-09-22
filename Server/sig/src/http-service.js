/*
* Http服务类
* */

// http服务
const Http = require('http');
Http.globalAgent.maxSockets = Infinity;
const Https = require('https');
const Cors = require('@koa/cors');
// 异步框架
const Koa = require('koa');
const Range = require('koa-range');
// 静态资源
const Serve = require('koa-static');
const BodyParser = require('koa-bodyparser');
// 公共库
const Fs = require('fs');
const Path = require('path');

// 项目公共库
const Common = require('./lib/common');
const Session = require('./lib/session');
const AppConfig = require('./config/app-config');
// 路由
const Router = require('koa-router');
// 项目接口
const mainRouter = require('./router/http/proxy-router');

// module.exports = class HttpService {
class HttpService {
    constructor() {
        Common.AppGlobalVar.rootPath = Path.join(__dirname);

        // 创建异步框架
        this.app = new Koa();
        this.app.use(Cors());
        this.app.use(Range);

        // 配置静态资源文件
        let staticRoot = new Serve(Path.join(__dirname, 'static'));
        this.app.use(staticRoot);

        // 使用session中间件
        this.app.use(Session.getSession());
        this.app.use(BodyParser());
        // 增加公共处理
        this.app.use(async function httpMethod(ctx, next) {
            if( Common.isNull(ctx.session.sessionId) ) {
                ctx.session = {
                    sessionId: 'USERID-' + Math.random().toString(36).substr(2).toLocaleUpperCase(),
                    count: 0,
                }
            } else {
                ctx.session.count++;
            }

            let ip = ctx.req.headers["x-orig-ip"];
            if (Common.isNull(ip)) {
                ip = ctx.request.ip;
            }
            Common.log('http', 'info','[' + ctx.session.sessionId + ']-request,' + ' (' + ctx.session.count + '), ' + ip + ', ' + ctx.request.url + ', ' + ctx.req.method + ', ' + JSON.stringify(ctx.req.headers));

            let start = process.uptime() * 1000;
            // 等待其他中间件处理的异步返回
            await next();
            // 所有中间件处理完成

            try {
                let end = process.uptime() * 1000;
                ctx.response._body.time = end - start + 'ms';
                let json = JSON.stringify(ctx.response._body);
                let desc = (json.length < 1024)?json:json.substring(0, 1024)+'...';
                Common.log('http', 'info','[' + ctx.session.sessionId + ']-response,' + ' (' + ctx.session.count + '), ' + ip + ', ' + ctx.request.url + ', ' + ctx.req.method + ', ' + desc);
            } catch (e) {
                Common.log('http', 'warn', '[' + ctx.session.sessionId + ']-response, ' +  ' (' + ctx.session.count + '), ' + ip + ', ' + ctx.request.url + ', ' + ctx.req.method + ', ' + e.toString());
            }
        });

        // 增加路由
        this.app.use(mainRouter.routes());
    }

    start(opts) {
        // 启动服务器
        opts = opts || {};

        // let port = AppConfig.http.port;
        // if( !Common.isNull(opts.number) ) {
        //     port += parseInt(opts.number);
        // }
        // AppConfig.http.port = port;
        Http.createServer(this.app.callback()).listen(AppConfig.http.port);

        // SSL options
        var options = {
            key: Fs.readFileSync('./etc/server.key'),  // ssl文件路径
            cert: Fs.readFileSync('./etc/server.crt')  // ssl文件路径
        };
        Https.createServer(options, this.app.callback()).listen(AppConfig.https.port);

        Common.log('http', 'fatal', 'Http service start in port : ' + AppConfig.http.port + ', Https service start in port : ' + AppConfig.https.port);
    }
}

function handle(signal) {
    Common.log('main', 'fatal', 'Http exit pid : ' + process.pid + ', env : ' + process.env.NODE_ENV);
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

// 启动Http
http = new HttpService();
http.start({number:number});