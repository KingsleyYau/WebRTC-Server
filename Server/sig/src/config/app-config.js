/*
* App服务内部端口配置
* Author: Max.Chiu
* */

// 数据库配置
const AppConfig = {
    log:{
        host: 'ws://127.0.0.1',     // 服务ip地址
        port: 9875                  // 端口
    },
    http:{
        host: 'http://127.0.0.1',   // 服务ip地址
        port: 9876                  // 端口
    },
    https:{
        host: 'https://127.0.0.1',  // 服务ip地址
        port: 9877                  // 端口
    },
    exApp:{
        host: 'ws://127.0.0.1',     // 服务ip地址
        port: 9776                 // 端口
    },
    inApp:{
        host: 'ws://127.0.0.1',     // 服务ip地址
        port: 9778                 // 起始端口
    },
    exApps:{
        host: 'wss://127.0.0.1',     // 服务ip地址
        port: 9777                 // 端口
    },
    exAppFlv:{
        host: 'wss://127.0.0.1',     // 服务ip地址
        port: 9779                 // 端口
    },
    proxy:{
        port: 9081,                             // 端口
        proxyHost: 'ws://192.168.88.133:9881',   // 代理服务ip地址
    },
    ice:[
        // 'turn://198.211.27.71:3478?transport=tcp',
        'turn://198.211.27.71:3478?transport=udp',
        // 'turn://192.168.88.133:3478',
    ],
    python:{
        // pd: "source /root/miniconda3/bin/activate pd && cd /root/project",
        // pd: "source /Users/max/Documents/tools/miniconda3/bin/activate pd && cd /Users/max/Documents/Project/Demo/python/pd",
        pd: "source /root/miniconda2/bin/activate pd && cd /root/Max/project",
        rok: "/root/Max/project/rok/web/rok_deamon.sh",
        rok_api: "source /root/miniconda2/bin/activate rok && cd /root/Max/project/rok",
        sh: "/root/Max/project/",
    },
}

module.exports = AppConfig;