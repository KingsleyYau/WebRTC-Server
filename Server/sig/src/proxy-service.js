/*
* 代理服务
* */

// 项目公共库
const Common = require('./lib/common');

function handle(signal) {
    Common.log('main', 'fatal', 'Proxy service exit pid : ' + process.pid + ', env : ' + process.env.NODE_ENV);
    // 退出程序
    process.exit();
}

process.on('SIGINT', handle);
process.on('SIGTERM', handle);
process.on('SIGTERM', handle);

Common.log('main', 'fatal', 'Proxy service start finish, pid : ' + process.pid + ', env : ' + process.env.NODE_ENV);