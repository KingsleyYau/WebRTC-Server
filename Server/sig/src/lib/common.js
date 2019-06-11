/*
* 项目公共库
* */

// 日志
const appLog = require('./app-log').AppLog.getInstance();
// App配置
const AppConfig = require('../config/app-config');
// Model的Keys
const DBModelKeys = require('../db/model-keys');

isNull = function(obj) {
    if( typeof(obj)!="undefined" && obj!=null ) {
        return false;
    }

    return true;
}

log = function(category, level, msg) {
    appLog.log(category, level, msg);
}

appInfo = function() {
    let serverHost = AppConfig.inApp.host;
    let serverPort = AppConfig.inApp.port;

    let appInfo = {
        serverHost:serverHost,
        serverPort:serverPort,
        serverUniquePattern:DBModelKeys.RedisKey.ServerOnlineCountKey + '-' + serverHost + ':' + serverPort,
    }

    return appInfo;
}

module.exports = {
    isNull,
    log,
    appInfo
}