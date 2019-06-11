/*
* redis管理类
* Author: Max.Chiu
* */

// Reids库
const Redis = require("redis");

// 公共库
const Common = require('./common');
// 数据库配置
const DBConfig = require('../config/db-config');

class RedisConnector {
    static getInstance() {
        if( Common.isNull(RedisConnector.instance) ) {
            RedisConnector.instance = new RedisConnector();
        }
        return RedisConnector.instance;
    }

    constructor() {
        Common.log('common', 'FATAL', 'Redis.start, ' + DBConfig.redis.host + ':' + DBConfig.redis.port);

        this.client = Redis.createClient(DBConfig.redis.port, DBConfig.redis.host);

        this.client.on("ready", () => {
            Common.log('common', 'warn', 'Redis.ready, ' + DBConfig.redis.host + ':' + DBConfig.redis.port);
        });

        this.client.on("connect", () => {
            Common.log('common', 'warn', 'Redis.connect, ' + DBConfig.redis.host + ':' + DBConfig.redis.port);
        });

        this.client.on("reconnecting", () => {
            Common.log('common', 'warn', 'Redis.reconnecting, ' + DBConfig.redis.host + ':' + DBConfig.redis.port);
        });

        this.client.on("error", (err) => {
            Common.log('common', 'error', 'Redis.err, ' + DBConfig.redis.host + ':' + DBConfig.redis.port + ', error: ' + err);
        });

        this.client.on("end", () => {
            Common.log('common', 'warn', 'Redis.end, ' + DBConfig.redis.host + ':' + DBConfig.redis.port);
        });

        this.client.on("warning", (msg) => {
            Common.log('common', 'warn', 'Redis.warning, ' + DBConfig.redis.host + ':' + DBConfig.redis.port + ', msg: ' + msg);
        });
    }
}

RedisConnector.instance = null;

module.exports = {
    RedisConnector
}