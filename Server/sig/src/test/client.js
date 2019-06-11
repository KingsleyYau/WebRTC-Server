/*
* 压力测试客户端
* Author: Max.Chiu
* */

const WebSocket = require('ws');

console.log('# Test Start, argv.length: ' + process.argv.length);

let start = 0;
if( process.argv.length > 2 ) {
    start = parseInt(process.argv[2], 10);
}

let number = 1;
if( process.argv.length > 3 ) {
    number = parseInt(process.argv[3], 10);
}

for(let i = start; i < start + number; i++) {
    try {
        // const client = new WebSocket('ws://127.0.0.1:9877/');
        const client = new WebSocket('ws://192.168.88.133:9877/');
        // const client = new WebSocket('wss://demo.charmlive.com:442/');

        let user = 'max-' + i;
        client.on('open', function open() {
            client.send('{"route":"imLogin/login","req_data":{"userId":"' + user + '","token":"token123456","pagename":7},"id":0}');

        });

        client.on('error', (e) => {
            console.log(e);
        });

        client.on('message', (data) => {
            //console.log(data);
            client.close();
        });
    } catch (e) {
        console.log('Client ' + i + ', ' + e);
    }
}

console.log('# Test End');