/*
* redis管理类
* Author: Max.Chiu
* */

// Reids库
// const Redis = require("redis");
const Redis = require("ioredis");

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
        let cng = JSON.stringify(DBConfig.redis.server);
        let options = {};
        Object.assign(options, DBConfig.redis.options, {
            clusterRetryStrategy:(times) => {
                var delay = Math.min(times * 100, 1000);
                Common.log('common', 'warn', 'Redis.clusterRetryStrategy, delay: ' + delay + ', times: ' + times);
                return delay;
            },
            reconnectOnError:(err) => {
                Common.log('common', 'warn', 'Redis.reconnectOnError, err: ' + err);
            }
        });
        Common.log('common', 'FATAL', 'Redis.constructor, ' + cng);

        // this.client = Redis.createClient(DBConfig.redis.port, DBConfig.redis.host);
        this.client = new Redis.Cluster(
            DBConfig.redis.server,
            options
        );

        this.client.on("ready", () => {
            Common.log('common', 'warn', 'Redis.ready, ' + cng);
        });

        this.client.on("connect", () => {
            Common.log('common', 'warn', 'Redis.connect, ' + cng);
        });

        this.client.on("reconnecting", () => {
            Common.log('common', 'warn', 'Redis.reconnecting, ' + cng + this);
        });

        this.client.on("error", (err) => {
            Common.log('common', 'error', 'Redis.err, ' + cng + ', error: ' + err);
        });

        this.client.on("end", () => {
            Common.log('common', 'warn', 'Redis.end, ' + cng);
        });

        this.client.on("warning", (msg) => {
            Common.log('common', 'warn', 'Redis.warning, ' + cng + ', msg: ' + msg);
        });
    }
}

RedisConnector.instance = null;

module.exports = {
    RedisConnector
}