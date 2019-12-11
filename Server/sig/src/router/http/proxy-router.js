/*
* 获取代理接口
* Author: Max.Chiu
* */

// 路由
const Router = require('koa-router');

// 项目公共库
const Common = require('../../lib/common');

const Fs = require('fs');
const Path = require('path');

function readDirSync(path, httpPath){
    let json = [];
    let pa = Fs.readdirSync(path);
    pa.forEach(function(file, index){
        let info = Fs.statSync(path + "/" + file)
        if( info.isDirectory() ){
            // readDirSync(path + "/"+ file);
        } else {
            let absolutePath = path + "/" + file;
            let relativePath = httpPath + "/" + file;
            // console.log("absolutePath: ". absolutePath, ", relativePath: ", relativePath);

            let rex = /.*(.jpg)/;
            let bFlag = rex.test(relativePath);
            if ( bFlag ) {
                json.push(relativePath);
            }
        }
    })
    return json;
}

// 设置路由
let proxyRouter = new Router();
// 定义为异步中间件
proxyRouter.all('/serverList', async (ctx, next) => {
    let respond = [
        {host:'127.0.0.1', port:9877},
        {host:'127.0.0.1', port:9878}
        ];
    ctx.body = respond;
});

proxyRouter.all('/verify/v1/start', async (ctx, next) => {
    let respond =
        {"errno":0,"errmsg":""}
    ;
    ctx.body = respond;
});

proxyRouter.all('/snapshot', async (ctx, next) => {
    let respond = readDirSync(Common.AppGlobalVar.rootPath + "/static/snapshot", "snapshot");
    ctx.body = respond;
});

proxyRouter.all('/snapshot_backup', async (ctx, next) => {
    let respond = readDirSync(Common.AppGlobalVar.rootPath + "/static/snapshot_backup", "snapshot_backup");
    ctx.body = respond;
});

// proxyRouter.all('/test', (ctx, next) => {
//     // 等待异步接口
//     let respond = 'test';
//     ctx.body = respond;
// });

module.exports = proxyRouter;