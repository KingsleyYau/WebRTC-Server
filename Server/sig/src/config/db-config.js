/*
* [Mysql/Redis]数据库配置
* Author: Max.Chiu
* */

// 数据库配置
const DBConfig = {
    mysql:{
        database: 'root',       // 数据库
        username: 'root',       // 用户
        password: 'aS626815!',  // 密码
        host: '39.106.12.86',   // 服务ip地址
        port: '3306'           // 端口
    },
    redis:{
        server:[
            {
                // host: '127.0.0.1',      // 服务ip地址
                host: '192.168.88.133',      // 服务ip地址
                // port: 6379              // 端口
                port: 7000              // 端口
            },
            {
                // host: '127.0.0.1',      // 服务ip地址
                host: '192.168.88.133',      // 服务ip地址
                // port: 6379              // 端口
                port: 7001              // 端口
            },
            {
                // host: '127.0.0.1',      // 服务ip地址
                host: '192.168.88.133',      // 服务ip地址
                // port: 6379              // 端口
                port: 7002              // 端口
            },
        ],
        options:{
            natMap: {
                "127.0.0.1:7000": { host: "192.168.88.133", port: 7000 },
                "127.0.0.1:7001": { host: "192.168.88.133", port: 7001 },
                "127.0.0.1:7002": { host: "192.168.88.133", port: 7002 },
                "127.0.0.1:7050": { host: "192.168.88.133", port: 7050 },
                "127.0.0.1:7051": { host: "192.168.88.133", port: 7051 },
                "127.0.0.1:7052": { host: "192.168.88.133", port: 7052 },
            },
            /**
             * 1."all": Send write queries to masters and read queries to masters or slaves randomly.
             " 2. "slave": Send write queries to masters and read queries to slaves.
             * 3. a custom function(nodes, command): node: Will choose the custom function to select to which node to send read queries (write queries keep being sent to master). The first node in nodes is always the master serving the relevant slots. If the function returns an array of nodes, a random node of that list will be selected.
             */
            scaleReads: "slave",
        }
    },
}

module.exports = DBConfig;