/*
* 在线用户管理类
* Author: Max.Chiu
* */

// 项目公共库
const Common = require('../lib/common');
// 用户管理器
const Users = require('./users');
// Redis
// const redisClient = require('../lib/redis-connector').RedisConnector.getInstance();
// Model的Keys
const DBModelKeys = require('../db/model-keys');

class OnlineUserManager {
    static getInstance() {
        if( Common.isNull(OnlineUserManager.instance) ) {
            OnlineUserManager.instance = new OnlineUserManager();
        }
        return OnlineUserManager.instance;
    }

    constructor() {
        this.userManager = new Users.UserManager();
    }

    /*
    * 增加用户
    * 如果已经登录, 则通知其他进程踢出, 再登录
    *
    * */
    async login(user) {
        return new Promise( async (resolve, reject) => {
            Common.log('im', 'warn', '[' + user.userId  + ']-OnlineUserManager.login, [用户登录], [' + user.socketId + ']');

            // 保存登录信息
            // this.setUserToRedis(user, resolve);

            // 保存到本地
            this.userManager.addUser(user);
            resolve();
        });
    }

    /*
    * 删除用户
    * */
    async logout(user) {
        // 本地删除
        return new Promise( async (resolve, reject) => {
            if( !Common.isNull(user) ) {
                this.userManager.delUser(user.socketId);

                // // redis删除
                // redisClient.client.del(user.uniquePattern(), (err, res) => {
                //     Common.log('im', 'warn', '[' + user.userId  + ']-OnlineUserManager.logout, [用户注销], [' +  user.socketId +  '], delete: ' + res + ', err: ' + err);
                //     resolve();
                // });
            } else {
                Common.log('im', 'warn', '[null]-OnlineUserManager.logout, No Such User');
                resolve();
            }
        });
    }

    /*
    * 根据SocketId获取本地用户
    * @param socketId 连接唯一Id
    * */
    getUserWithSocket(socketId) {
        return this.userManager.getUser(socketId);
    }

    /*
    * 根据UserId获取唯一用户
    * */
    async getUserWithId(userId) {
        return new Promise( async (resolve, reject) => {
            // 本地返回
            let user = this.userManager.getUserWithId(userId);
            let userList = [];
            if ( !Common.isNull(user) ) {
                userList.push(user);
            }

            if ( userList.length > 0 ) {
                resolve(userList);
            } else {
                reject();
            }

            // redisClient.client.keys(Users.User.userIdPattern(userId), async (err, res) => {
            //     let userList = [];
            //     if( !Common.isNull(res) && res.length > 0 ) {
            //         for(let i = 0; i < res.length; i++){
            //             // 如果用户已经登录
            //             await new Promise( async (resolve, reject) => {
            //                 redisClient.client.hgetall(res[i], (err, res) => {
            //                     if( res != null ) {
            //                         // 获取用户登录信息成功, 需要踢掉旧的连接
            //                         let json = JSON.stringify(res);
            //                         let user = this.getUserWithRedis(res);
            //                         userList.push(user);
            //                         resolve(user);
            //                     }
            //                 });
            //             });
            //         }
            //     }
            //     // Common.log('im', 'debug', '[' + userId  + ']-OnlineUserManager.getUserWithId, hgetall, userList: ' + userList.length + ', err: ' + err);
            //     resolve(userList);
            // });
        });
    }

    /*
    * 清空本地所有用户
    * */
    logoutAllLocalUsers() {
        Common.log('im', 'warn', '[server]OnlineUserManager.logoutAllLocalUsers');

        this.userManager.getUsers((userId, user) => {
            // redis删除
            // redisClient.client.del(user.uniquePattern(), null);
        });
    }

    /*
    * 保存用户到Redis
    * */
    setUserToRedis(user, resolve) {
        // 增加到redis
        // redisClient.client.hmset(user.uniquePattern(),
        //     DBModelKeys.RedisKey.UserKey.SocketIdKey, user.socketId,
        //     DBModelKeys.RedisKey.UserKey.UserIdKey, user.userId,
        //     DBModelKeys.RedisKey.UserKey.ConnectTimeKey, user.connectTime,
        //     DBModelKeys.RedisKey.UserKey.LoginTimeKey, user.loginTime,
        //     DBModelKeys.RedisKey.UserKey.ServerHostKey, user.serverHost,
        //     DBModelKeys.RedisKey.UserKey.ServerPortKey, user.serverPort,
        //     (err, res) => {
        //         Common.log('im', 'debug', '[' + user.userId  + ']-OnlineUserManager.setUserToRedis, [' + user.socketId + '], ' + res + ', err:' + err);
        //
        //         // 增加本地用户
        //         this.userManager.addUser(user);
        //
        //         // 登录处理完成
        //         resolve();
        //     });
    }

    /*
    * 转换Redis到用户结构体
    * */
    getUserWithRedis(res) {
        let user = this.userManager.getUser(res.SocketIdKey);
        if( Common.isNull(user) ) {
            user = Users.User.createUserWithRedis(
                res.UserIdKey,
                res.SocketIdKey,
                res.ConnectTimeKey,
                res.LoginTimeKey,
                res.ServerHostKey,
                res.ServerPortKey,
            );
        }
        return user;
    }
}
OnlineUserManager.instance = null;

module.exports = {
    OnlineUserManager
}