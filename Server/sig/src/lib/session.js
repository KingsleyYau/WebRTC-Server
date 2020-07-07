const Session = require('koa-session-minimal')
// const MysqlSession = require('koa-mysql-user')
//
// let store = new MysqlSession({
//     user: 'root',
//     password: 'abc123',
//     database: 'koa_demo',
//     host: '127.0.0.1',
// })

// 存放sessionId的cookie配置
const session = {
    maxAge: 300 * 1000, // cookie有效时长
    expires: '',  // cookie失效时间
    path: '', // 写cookie所在的路径
    domain: '', // 写cookie所在的域名
    httpOnly: '', // 是否只用于http请求中获取
    overwrite: '',  // 是否允许重写
    secure: '',
    sameSite: '',
    signed: '',
}

exports.getSession = function () {
    return Session({
        key: 'SESSION_ID',
        store: '',
        cookie: session
    });
}