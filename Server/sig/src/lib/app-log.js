const io = require('socket.io-client');
const jsonParser = require('socket.io-json-parser');

// 公共库
const Log = require('./log');
const AppConfig = require('../config/app-config');

class AppLog {
    static getInstance() {
        if( typeof(AppLog.instance)=="undefined" || AppLog.instance==null ) {
            AppLog.instance = new AppLog();
        }
        return AppLog.instance;
    }

    constructor() {
        this.client = io(AppConfig.log.host + ':' + AppConfig.log.port, {
            parser:jsonParser
        });
    }

    log(category, level, msg) {
        if( typeof(process.env.NODE_ENV) == "undefined" ) {
            // 本地打印日志
            let logger = Log.getLogger(category);
            logger.log(level, msg);

        } else {
            // 远程服务打印日志
            this.logRemote(category, level, msg);
        }
    }

    logRemote(category, level, msg) {
        this.client.emit('log', {
            pid:process.pid,
            category:category,
            level:level,
            msg:msg
        });
    }
}

AppLog.instance = null;

module.exports = {
    AppLog
}