/*
* [Mysql/Redis]数据库配置
* Author: Max.Chiu
* */

// 数据库配置
const DBConfig = {
    mysql:{
        database: 'root',       // 数据库
        username: 'root',       // 用户
        password: 'aS626815!',  // 密码
        host: '39.106.12.86',   // 服务ip地址
        port: '3306'           // 端口
    },
    redis:{
        username: 'root',       // 用户
        password: 'aS626815!',  // 密码
        host: '127.0.0.1',      // 服务ip地址
        port: 6379              // 端口
    }
}

module.exports = DBConfig;