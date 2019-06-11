/*
* 允许处理的路由
* Author: Max.Chiu
* */

// 项目公共库
const Common = require('../../../../lib/common');
// 业务逻辑处理
const HeartBeatHandler = require('./heartbeat-handler');
const LoginHandler = require('./login-handler');
const LogoutHandler = require('./logout-handler');
const SendMsgHandler = require('./sendmsg-handler');
const SendSdpCallHandler = require('./sendsdpcall-handler');
const SendSdpAnswerHandler = require('./sendsdpanswer-handler');

class HandleRouter {
    static getInstance() {
        if( Common.isNull(HandleRouter.instance) ) {
            HandleRouter.instance = new HandleRouter();
        }
        return HandleRouter.instance;
    }

    constructor() {
        this.routeArray = this.constructor.getAllRoutes();
        this.routeMap = {};
        for(let i = 0; i < this.routeArray.length; i++) {
            let router = this.routeArray[i];
            this.routeMap[router.getRoute()] = router;
        }
    }

    static getAllRoutes() {
        return [
            HeartBeatHandler,
            LoginHandler,
            LogoutHandler,
            SendMsgHandler,
            SendSdpCallHandler,
            SendSdpAnswerHandler,
        ]
    }

    getRouterByName(name) {
        return this.routeMap[name];
    }
}

HandleRouter.instance = null;

module.exports = {
    HandleRouter
}