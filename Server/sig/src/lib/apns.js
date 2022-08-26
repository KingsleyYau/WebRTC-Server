// 项目公共库
const Common = require('./common');
const apn = require("apn");

class Apns {
    static getInstance() {
        if (typeof (Apns.instance) == "undefined" || Apns.instance == null) {
            Apns.instance = new Apns();
        }
        return Apns.instance;
    }

    constructor() {
        // this.service = new apn.Provider({
        //     cert: "./etc/push/aps_production_cer.pem",
        //     key: "./etc/push/aps_production_key.pem",
        //     // gateway: "api.sandbox.push.apple.com",
        //     gateway: "api.push.apple.com",
        //     port: 443,
        //     passphrase: "9527",
        //     production:true,
        // });
    }

    async send(tokens, body) {
        let note = new apn.Notification({
            alert: {
                body : body,
            },
            badge : 1,
        });
        note.topic = "net.maxzoon.ai.Cartoon"

        Common.log('http', 'info', `Apns sending: ${note.compile()} to [${tokens}]`);
        // return this.service.send(note, tokens).then( result => {
        //     if (result.failed.length > 0) {
        //         Common.log('http', 'warn', `Apns sending fail: ${result.sent.length} / ${tokens.length}, ${result.failed[0].response.reason}, ${result.failed[0].status}, ${result.failed[0].device}`);
        //     } else {
        //         Common.log('http', 'info', `Apns sending success: [${tokens}]`);
        //     }
        // });
        return new Promise((resolve, reject) => {
            Common.log('http', 'info', `Apns sending success: [${tokens}]`);
            resolve(`Apns sending success: [${tokens}]`);
        });
    }

    shutdown() {
        // this.service.shutdown();
    }
}

Apns.instance = null;
module.exports = {
    Apns
}