/*
* 获取代理接口
* Author: Max.Chiu
* */

// 路由
const Router = require('koa-router');

// 项目公共库
const Common = require('../../lib/common');

// 设置路由
let proxyRouter = new Router();
// 定义为异步中间件
proxyRouter.all('/serverList', async (ctx, next) => {
    // 等待异步接口
    let respond = [
        {host:'127.0.0.1', port:9877},
        {host:'127.0.0.1', port:9878}
        ];
    ctx.body = respond;
});

// proxyRouter.all('/test', (ctx, next) => {
//     // 等待异步接口
//     let respond = 'test';
//     ctx.body = respond;
// });

module.exports = proxyRouter;