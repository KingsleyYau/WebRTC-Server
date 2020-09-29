/*
* 用户管理类
* Author: Max.Chiu
* */

// 项目公共库
const Common = require('../lib/common');
// App配置
const AppConfig = require('../config/app-config');
// Model的Keys
const DBModelKeys = require('../db/model-keys');

class User {
    constructor(userId, socketId, websocket, connectTime, loginTime) {
        this.noticeId = 0;
        this.userId = userId;
        this.socketId = socketId;
        this.websocket = websocket;
        this.connectTime = connectTime;
        this.loginTime = loginTime;
        this.serverHost = '';
        this.serverPort = 0;
    }

    static createUserWithLogin(userId, socketId, websocket, connectTime, loginTime) {
        let user = new this(userId, socketId, websocket, connectTime, loginTime);

        let appInfo = Common.appInfo();
        user.serverHost = appInfo.serverHost;
        user.serverPort = appInfo.serverPort;

        return user;
    }

    static createUserWithRedis(userId, socketId, connectTime, loginTime, serverHost, serverPort) {
        let user = new this(userId, socketId, null, connectTime, loginTime);

        user.serverHost = serverHost;
        user.serverPort = serverPort;

        return user;
    }

    static createUserWithId(userId) {
        return new this(userId, null, null, null, null, null);
    }

    static userIdPattern(userId) {
        return DBModelKeys.RedisKey.OnlineUserKey + '-' + userId + '-*';
    }

    uniquePattern() {
        return DBModelKeys.RedisKey.OnlineUserKey + '-' + this.userId + '-' + this.socketId;
    }
}

class UserManager {
    constructor() {
        this.userList = {};
        this.userMap = {};
    }

    addUser(user) {
        this.userList[user.socketId] = user;
        this.userMap[user.userId] = user;
    }

    delUser(socketId) {
        delete this.userMap[this.userList[socketId].userId];
        delete this.userList[socketId];
    }

    getUser(socketId) {
        return this.userList[socketId];
    }

    getUserWithId(userId) {
        return this.userMap[userId];
    }

    getUsers(cb) {
        Object.keys(this.userList).forEach((socketId) => {
            cb(socketId, this.userList[socketId]);
        });
    }
}

module.exports = {
    User,
    UserManager
}