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
// const md5 = (content) => encrypt('md5', content);


// 定义 MD5 计算函数
function md5(str) {
    // 创建 MD5 哈希对象
    const hash = Crypto.createHash('md5');
    // 更新哈希内容（支持字符串或 Buffer）
    hash.update(str, 'utf8'); // 第二个参数指定编码（默认 utf8）
    // 输出十六进制结果（32位字符串）
    return hash.digest('hex');
}

/**
 * 签名生成器 - 为API请求生成签名以确保请求的安全性
 * @param {Object} params - 请求参数对象
 * @returns {Object} - 添加了时间戳和签名后的参数对象
 */
function generateSign(params) {
    // 添加当前时间戳作为参数的一部分
    params.t = Date.now();
    // 生成签名
    params.sign = createSign(params, "rok-server-lujun");
    return params;
}

/**
 * 创建签名的核心函数
 * @param {Object} params - 要签名的参数对象
 * @param {string} salt - 签名盐值，增加安全性
 * @returns {string} - MD5签名结果
 */
function createSign(params, salt = "") {
    // 过滤出除了sign以外的所有参数键名
    const paramKeys = Object.keys(params).filter(key => key !== "sign");
    // 对参数键名进行字典序排序
    paramKeys.sort();
    // 构建参数字符串，格式为"key1=value1&key2=value2..."
    const paramString = paramKeys.map(key => {
        return `${key}=${params[key]}`;
    }).join("&") + salt;
    // 获取MD5工具并计算签名
    return md5(paramString)
}

module.exports = {
    isNull,
    log,
    appInfo,
    AppGlobalVar,
    sha1,
    md5,
    generateSign
}