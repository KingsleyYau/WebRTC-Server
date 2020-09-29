/*
* 项目公共库
* */

// 日志
const appLog = require('./app-log').AppLog.getInstance();
// App配置
const AppConfig = require('../config/app-config');
// Model的Keys
const DBModelKeys = require('../db/model-keys');
// OpenSSL
const Crypto = require('crypto');

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

let AppGlobalVar = {
    rootPath:""
}

const encrypt = (algorithm, content, key, encoding) => {
    let hash = Crypto.createHmac(algorithm, key);
    hash.update(content);
    return hash.digest(encoding);
}

const sha1 = (content, key) => encrypt('sha1', content, key);
const md5 = (content) => encrypt('md5', content);

module.exports = {
    isNull,
    log,
    appInfo,
    AppGlobalVar,
    sha1,
    md5
}