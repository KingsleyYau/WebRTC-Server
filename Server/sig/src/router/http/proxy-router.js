/*
* 获取代理接口
* Author: Max.Chiu
* */

// 路由
const Router = require('koa-router');

// 项目公共库
const Common = require('../../lib/common');
const AppConfig = require('../../config/app-config');
const apns = require('../../lib/apns').Apns.getInstance();

// Redis
// const redisClient = require('../../lib/redis-connector').RedisConnector.getInstance();
// Model的Keys
const DBModelKeys = require('../../db/model-keys');

const fs = require('fs');
const path = require('path');
const url = require('url');
const exec = require('child_process');
const formidable = require('formidable');
const mime = require('mime-types');
const querystring = require('querystring');

function readDirSync(path, httpPath){
    let json = [];
    let pa = fs.readdirSync(path);
    pa.forEach(function(file, index){
        let info = fs.statSync(path + "/" + file)
        if( info.isDirectory() ){
            // readDirSync(path + "/"+ file);
        } else {
            let absolutePath = path + "/" + file;
            let relativePath = httpPath + "/" + file;
            // console.log("absolutePath: ". absolutePath, ", relativePath: ", relativePath);

            let rex = /.*(.jpg|.jpeg|.png)/;
            let bFlag = rex.test(relativePath.toLowerCase());
            if ( bFlag ) {
                json.push(relativePath);
            }
        }
    })
    return json;
}

// 设置路由
let proxyRouter = new Router();
// 定义为异步中间件
proxyRouter.all('/serverList', async (ctx, next) => {
    let respond = [
        {host:'127.0.0.1', port:9877},
        {host:'127.0.0.1', port:9878}
        ];
    ctx.body = respond;
});

proxyRouter.all('/verify/v1/start', async (ctx, next) => {
    let respond =
        {"errno":0,"errmsg":""}
    ;
    ctx.body = respond;
});

proxyRouter.all('/sync', async (ctx, next) => {
    let respond;

    exec.exec('cd /root/Github/LiveServer/doc && ./autologin.sh && ./preview_8899.sh', (err, stdout, stderr) => {
        if ( err || stderr ) {
            respond = stdout;
        } else {
            respond = 'OK';
        }
    })

    ctx.body = respond;
});

proxyRouter.all('/snapshot', async (ctx, next) => {
    let respond = readDirSync(Common.AppGlobalVar.rootPath + "/static/snapshot", "snapshot");
    ctx.body = respond;
});

proxyRouter.all('/snapshot_backup', async (ctx, next) => {
    let respond = readDirSync(Common.AppGlobalVar.rootPath + "/static/snapshot_backup", "snapshot_backup");
    ctx.body = respond;
});

proxyRouter.all('/pic_jpg', async (ctx, next) => {
    let respond = readDirSync(Common.AppGlobalVar.rootPath + "/static/pic_jpg", "pic_jpg");
    ctx.body = respond;
});

proxyRouter.all('/snapshot_camshare', async (ctx, next) => {
    let respond = readDirSync(Common.AppGlobalVar.rootPath + "/static/snapshot_camshare", "snapshot_camshare");
    ctx.body = respond;
});

proxyRouter.all('/set', async (ctx, next) => {
    let respond = {
        "errno":0,
        "errmsg":"",
        "res":-2,
        "userId":ctx.session.sessionId,
    }

    // 增加到redis
    // await redisClient.client.
    // hsetnx(
    //     'hash_user_online_' + ctx.session.sessionId,
    //     'name', ctx.session.sessionId
    // ).then( (res) => {
    //     Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-hsetnx], res: ' + res);
    //     respond.res = res;
    // }).catch( (err) => {
    //     Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-hsetnx], err: ' + err);
    //     respond.errmsg = err;
    // });
    //
    // await redisClient.client.
    // expire(
    //     'hash_user_online_' + ctx.session.sessionId,
    //     300
    // ).then( (res) => {
    //     Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-expire], res: ' + res);
    //     respond.res = res;
    // }).catch( (err) => {
    //     Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-expire], err: ' + err);
    //     respond.errmsg = err;
    // });

    let start = process.uptime() * 1000;

    // 随机增加时间, 防止雪崩
    let timeRnd = Math.floor(Math.random() * 30);
    // 因为使用集群, 必须保证key的hash在同一个slot, 否则不能使用事务
    // await redisClient.client.multi().
    // hset(
    //     'h_user_online_' + ctx.session.sessionId,
    //     'name', 'max-' + ctx.session.sessionId,
    //     'age', 18
    // ).expire(
    //     'h_user_online_' + ctx.session.sessionId,
    //     1800 + timeRnd
    // ).exec().then( (res) => {
    //     let all = JSON.stringify(res);
    //     Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-set], res:' + all);
    //     respond.res = res;
    // }).catch( (err) => {
    //     Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-set], err:' + err);
    //     respond.errmsg = err;
    // });
    let end = process.uptime() * 1000;
    respond.time = end - start + 'ms';

    ctx.body = respond;
});

proxyRouter.all('/get', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        res:-2,
        user_id:ctx.session.sessionId,
    }

    let url = url.parse(decodeURI(ctx.originalUrl.toLowerCase()),true);
    let user_id = url.query.user_id;
    let start = process.uptime() * 1000;
    // 可以在这里增加hash filter, 减少缓存穿透
    await redisClient.client.multi().
    hgetall(
        'hash_user_online_' + user_id
    ).exec().then( (res) => {
        let all = JSON.stringify(res);
        Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-get], res:' + all);
        respond.res = res;
    }).catch( (err) => {
        Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-get], err:' + err);
        respond.errmsg = err;
    });
    let end = process.uptime() * 1000;
    respond.time = end - start + 'ms';

    ctx.body = respond;
});

proxyRouter.all('/nodes', async (ctx, next) => {
    let respond = {
        "errno":0,
        "errmsg":"",
        "res":-2,
        "userId":ctx.session.sessionId,
    }

    let start = process.uptime() * 1000;
    let nodes = redisClient.client.nodes('slave');
    await Promise.all(
        nodes.map(function (node) {
           return node.keys('*');
        })
    ).then( (res) => {
        // respond.res = res;
    });
    await Promise.all(
        nodes.map(function (node) {
            return node.dbsize();
        })
    ).then( (res) => {
        respond.res = res;
    });
    let end = process.uptime() * 1000;
    respond.time = end - start + 'ms';

    ctx.body = respond;
});

proxyRouter.all('/nodes_dbsize', async (ctx, next) => {
    let respond = {
        "errno":0,
        "errmsg":"",
        "res":-2,
        "userId":ctx.session.sessionId,
    }

    let start = process.uptime() * 1000;
    let nodes = redisClient.client.nodes('slave');
    await Promise.all(
        nodes.map(function (node) {
            return node.dbsize();
        })
    ).then( (res) => {
        respond.res = res;
    });
    let end = process.uptime() * 1000;
    respond.time = end - start + 'ms';

    ctx.body = respond;
});

proxyRouter.all('/rnd', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        res:-2,
        userId:ctx.session.sessionId,
    }

    let start = process.uptime() * 1000;

    // 可以在这里增加hash filter, 减少缓存穿透
    let nodes = redisClient.client.nodes('slave');
    let index = Math.floor(Math.random() * 10) % nodes.length;
    let cursor = -1;
    await nodes[index].dbsize().then( (res) => {
        if ( res > 0 ) {
            cursor = Math.floor(Math.random() * 10) % res;
        }
    }).catch( (err) => {
        Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-rnd], err:' + err);
    });

    if ( cursor > -1 ) {
        Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-rnd], index:' + index + ', cursor:'+ cursor);
        await nodes[index].multi().
        scan(
            cursor, 'match', 'hash_user_online_*', 'count', 10
        ).exec().then( (res) => {
            let all = JSON.stringify(res);
            Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-rnd], res:' + all);
            respond.res = res;
        }).catch( (err) => {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-rnd], err:' + err);
            respond.errmsg = err;
        });
    }

    let end = process.uptime() * 1000;
    respond.time = end - start + 'ms';

    ctx.body = respond;
});

const P2C = AppConfig.python.pd + ' && python p2c_arg.py'
proxyRouter.all('/upload', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
            path:"",
            photo:"",
            cartoon:""
        }
    }

    let start = process.uptime() * 1000;
    let end = process.uptime() * 1000;
    respond.time = end - start + 'ms';

    // ctx.session.data = new Array(1e7).join('*');
    let form = new formidable.IncomingForm();
    form.encoding = 'utf-8';
    form.uploadDir = path.join(__dirname + "../../../static/upload");
    form.keepExtensions = true;//保留后缀
    form.maxFieldsSize = 2 * 1024 * 1024;

    fs.mkdir(form.uploadDir, { recursive: true }, (err) => {
        if (err) {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-upload], err:' + err);
        }
    });

    let cartoon_dir = path.join(form.uploadDir, "cartoon");
    fs.mkdir(cartoon_dir, { recursive: true }, (err) => {
        if (err) {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-upload], err:' + err);
        }
    });

    await new Promise(function(resolve, reject) {
        form.parse(ctx.req, function (err, fields, files) {
            upload_file = "";
            try {
                let filepath = files.upload_file.path;
                let dir = path.dirname(filepath)
                let basename = path.basename(filepath)
                let basename_pre = basename.split('.')[0];

                let align_face = 1;
                if( fields.align_face == "0" ) {
                    align_face = 0;
                }

                let style = 0;
                if( !Common.isNull(fields.style)  ) {
                    style = fields.style;
                }

                let upload_path = "/upload/";
                let upload_file = upload_path + basename;
                Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-/upload], ' + upload_file);

                let photo_path = path.join(dir, basename_pre + "_photo.png");
                let cartoon_path = path.join(dir, basename_pre + "_cartoon.png");

                let cmd = P2C + ' --input_image ' + filepath + " --align_face " + align_face + ' --style ' + style
                // exec.execSync(cmd)
                child = exec.exec(cmd, function(error, stdout, stderr) {
                    if(error) {
                        Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/upload], ' + upload_file + ', ' + error.toString());
                        respond.errno = 1;
                        respond.errmsg = error.message;
                    } else {
                        try {
                            let data = fs.readFileSync(photo_path);
                            data = new Buffer(data).toString('base64');
                            let photo_base64 = 'data:' + mime.lookup(photo_path) + ';base64,' + data;
                            respond.data.photo = photo_base64//upload_path + basename_pre + "_photo.png";

                            data = fs.readFileSync(cartoon_path);
                            data = new Buffer(data).toString('base64');
                            let cartoon_base64 = 'data:' + mime.lookup(cartoon_path) + ';base64,' + data;

                            respond.data.cartoon = cartoon_base64//upload_path + basename_pre + "_cartoon.png";
                            respond.data.file_id = basename_pre.split('_')[1];

                            exec.exec('mv ' + photo_path  + ' ' + cartoon_dir)
                            exec.exec('mv ' + cartoon_path  + ' ' + cartoon_dir)
                        } catch (e) {
                            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/upload], ' + upload_file + ', ' + e.toString());
                            respond.errno = 1;
                            respond.errmsg = e.message;
                        }
                    }
                    resolve();
                });
                Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-/upload], ' + cmd + ", pid:" +  child.pid);

            } catch (e) {
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/upload], ' + upload_file + ', ' + e.toString());
                respond.errno = 1;
                respond.errmsg = "Process fail";
                // reject(e);
                resolve();
            } finally {
                // resolve();
            }
        })
    })

    ctx.body = respond;
});

const REALSR = AppConfig.python.pd + ' && python realsr_arg.py'
proxyRouter.all('/upload_realsr', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
            path:"",
            photo:"",
            cartoon:""
        }
    }

    let start = process.uptime() * 1000;

    // ctx.session.data = new Array(1e7).join('*');
    var form = new formidable.IncomingForm();
    form.encoding = 'utf-8';
    form.uploadDir = path.join(__dirname + "../../../static/upload_realsr");
    form.keepExtensions = true;//保留后缀
    form.maxFieldsSize = 2 * 1024 * 1024;

    fs.mkdir(form.uploadDir, { recursive: true }, (err) => {
        if (err) {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-upload_realsr], err:' + err);
        }
    });

    await new Promise(function(resolve, reject) {
        form.parse(ctx.req, function (err, fields, files) {
            upload_file = "";
            try {
                let filepath = files.upload_file.path;
                dir = path.dirname(filepath)
                basename = path.basename(filepath)
                basename_pre = basename.split('.')[0];
                basename_ext = basename.split('.')[1];

                upload_path = "/upload_realsr/";
                upload_file = upload_path + basename;
                Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-upload_realsr], ' + upload_file);

                let cmd = REALSR + ' --input_image ' + filepath
                exec.execSync(cmd)

                photo_path = path.join(dir, basename_pre + "_realsr." + basename_ext);

                data = fs.readFileSync(photo_path);
                data = new Buffer(data).toString('base64');
                photo_base64 = 'data:' + mime.lookup(photo_path) + ';base64,' + data;

                respond.data.photo = photo_base64//upload_path + basename_pre + "_photo.png";

            } catch (e) {
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-upload_realsr], ' + upload_file + ', ' + e.toString());
                respond.errno = 1;
                respond.errmsg = "Process fail";

            } finally {
                resolve();
            }
        })
    })
    let end = process.uptime() * 1000;
    respond.time = end - start + 'ms';

    ctx.body = respond;
});

proxyRouter.all('/api/upload_realsr', async (ctx, next) => {
    token = Math.random().toString(36).substr(2).toLocaleUpperCase();
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
            token:token
        }
    }

    let start = process.uptime() * 1000;

    // ctx.session.data = new Array(1e7).join('*');
    let form = new formidable.IncomingForm();
    form.encoding = 'utf-8';
    form.uploadDir = path.join(__dirname + "../../../static/upload_realsr");
    form.keepExtensions = true;//保留后缀
    form.maxFieldsSize = 2 * 1024 * 1024;

    fs.mkdir(form.uploadDir, { recursive: true }, (err) => {
        if (err) {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_realsr], err:' + err);
        }
    });

    await new Promise(function(resolve, reject) {
        form.parse(ctx.req, function (err, fields, files) {
            upload_file = "";
            try {

                let device_token = "";
                if ( !Common.isNull(ctx.req.headers["device-token"]) ) {
                    device_token = ctx.req.headers["device-token"];
                }

                let filepath = files.upload_file.path;
                let dir = path.dirname(filepath)
                let basename = path.basename(filepath)
                let basename_pre = basename.split('.')[0];
                let basename_ext = basename.split('.')[1];

                let upload_path = "upload_realsr";
                let upload_file = path.join(upload_path, basename);
                Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-/api/upload_realsr], ' + upload_file);

                let output_path = path.join(dir, basename_pre + "_realsr." + basename_ext);
                let progress_path = path.join(dir, token + ".txt");
                let relative_path = path.join(upload_path, basename_pre + "_realsr." + basename_ext);

                obj = {
                    progress:0,
                    path:relative_path,
                }

                let json = JSON.stringify(obj);
                fs.writeFile(progress_path, json, e => {
                    if (!e) {
                        let cmd = REALSR + ' --input_image ' + filepath + ' --progress_path ' + progress_path
                        child = exec.exec(cmd, function(error, stdout, stderr) {
                            if(error) {
                                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_realsr], ' + upload_file + ', ' + error.toString());
                                fs.unlink(progress_path);
                            } else {
                                if( device_token != "" ) {
                                    apns.send([device_token], "Congratulation! You have a new supervision photo.");
                                }
                            }
                        });
                        Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-/api/upload_realsr], ' + cmd + ", pid:" +  child.pid);
                    } else {
                        Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_realsr], ' + upload_file + ', ' + e.toString());
                    }
                    resolve();
                })

            } catch (e) {
                respond.errno = 1;
                respond.errmsg = "Process fail";
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_realsr], ' + upload_file + ', ' + e.toString());
                resolve();
            } finally {
                // resolve();
            }
        })
    })
    let end = process.uptime() * 1000;
    respond.time = end - start + 'ms';

    ctx.body = respond;
});

proxyRouter.all('/api/query_realsr', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {
            path: "",
            progress:0
        }
    }

    let params = querystring.parse(ctx.querystring);
    let token = "";
    if (!Common.isNull(params.token)) {
        token = params.token;
    }

    if (token.length > 0) {
        let dir = path.join(__dirname, "../../static/upload_realsr");
        let progress_path = path.join(dir, token + ".txt");
        await new Promise(function(resolve, reject) {
            fs.readFile(progress_path, 'utf8', (err, data) => {
                if (!err) {
                    let obj = JSON.parse(data);
                    respond.data = obj;
                } else {
                    Common.log('http', 'warn', '[' + ctx.session.sessionId + ']-query_realsr], ' + token + ', ' + err);
                    respond.errno = -1;
                    respond.errmsg = 'No such file.';
                }
                resolve();
            });
        });
    }
    ctx.body = respond;
});

const TOON_VIDEO = AppConfig.python.pd + ' && python p2c_video.py'
proxyRouter.all('/api/upload_toon_video', async (ctx, next) => {
    token = Math.random().toString(36).substr(2).toLocaleUpperCase();
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
            token:token
        }
    }

    let start = process.uptime() * 1000;

    // ctx.session.data = new Array(1e7).join('*');
    let form = new formidable.IncomingForm();
    form.encoding = 'utf-8';
    form.uploadDir = path.join(__dirname + "../../../static/upload_toon_video");
    form.keepExtensions = true;//保留后缀
    form.maxFieldsSize = 2 * 1024 * 1024;

    fs.mkdir(form.uploadDir, { recursive: true }, (err) => {
        if (err) {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_toon_video], err:' + err);
        }
    });

    await new Promise(function(resolve, reject) {
        form.parse(ctx.req, function (err, fields, files) {
            upload_file = "";
            try {

                let device_token = "";
                if ( !Common.isNull(ctx.req.headers["device-token"]) ) {
                    device_token = ctx.req.headers["device-token"];
                }

                let filepath = files.upload_file.path;
                let dir = path.dirname(filepath)
                let basename = path.basename(filepath)
                let basename_pre = basename.split('.')[0];
                let basename_ext = basename.split('.')[1];

                let upload_path = "upload_toon_video";
                let upload_file = path.join(upload_path, basename);
                Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-/api/upload_toon_video], ' + upload_file);

                let progress_path = path.join(dir, token + ".txt");
                let relative_path = path.join(upload_path, basename_pre + "_cartoon." + basename_ext);

                let obj = {
                    progress:0,
                    path:relative_path,
                }

                let json = JSON.stringify(obj);
                fs.writeFile(progress_path, json, e => {
                    if (!e) {
                        let cmd = TOON_VIDEO + ' --input_path ' + filepath + ' --progress_path ' + progress_path
                        let child = exec.exec(cmd, function(error, stdout, stderr) {
                            if(error) {
                                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_toon_video], ' + upload_file + ', ' + error.toString());
                                fs.unlink(progress_path, (e) => {

                                    }
                                );
                            } else {
                                if( device_token != "" ) {
                                    apns.send([device_token], "Congratulation! You have a new toon video.");
                                }
                            }
                        });
                        Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-/api/upload_toon_video], ' + cmd + ", pid:" +  child.pid);
                    } else {
                        Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_toon_video], ' + upload_file + ', ' + e.toString());
                    }
                    resolve();
                })

            } catch (e) {
                respond.errno = 1;
                respond.errmsg = "Process fail";
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_toon_video], ' + upload_file + ', ' + e.toString());
                resolve();
            } finally {
                // resolve();
            }
        })
    })
    let end = process.uptime() * 1000;
    respond.time = end - start + 'ms';

    ctx.body = respond;
});

proxyRouter.all('/api/query_toon_video', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {
            path: "",
            progress:0
        }
    }

    let params = querystring.parse(ctx.querystring);
    let token = "";
    if (!Common.isNull(params.token)) {
        token = params.token;
    }

    if (token.length > 0) {
        let dir = path.join(__dirname, "../../static/upload_toon_video");
        let progress_path = path.join(dir, token + ".txt");
        await new Promise(function(resolve, reject) {
            fs.readFile(progress_path, 'utf8', (err, data) => {
                if (!err) {
                    let obj = JSON.parse(data);
                    respond.data = obj;
                } else {
                    Common.log('http', 'warn', '[' + ctx.session.sessionId + ']-upload_toon_video], ' + token + ', ' + err);
                    respond.errno = -1;
                    respond.errmsg = 'No such file.';
                }
                resolve();
            });
        });
    }
    ctx.body = respond;
});

const BIG_MOUTH = AppConfig.python.pd + ' && python bigmouth_arg.py'
proxyRouter.all('/api/upload_bigmouth', async (ctx, next) => {
    let token = Math.random().toString(36).substr(2).toLocaleUpperCase();
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
            token:token
        }
    }

    let start = process.uptime() * 1000;

    // ctx.session.data = new Array(1e7).join('*');
    let form = new formidable.IncomingForm();
    form.encoding = 'utf-8';
    form.uploadDir = path.join(__dirname + "../../../static/upload_bigmouth");
    form.keepExtensions = true;//保留后缀
    form.maxFieldsSize = 2 * 1024 * 1024;

    fs.mkdir(form.uploadDir, { recursive: true }, (err) => {
        if (err) {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_bigmouth], err:' + err);
        }
    });

    await new Promise(function(resolve, reject) {
        form.parse(ctx.req, function (err, fields, files) {
            upload_file = "";
            try {
                let device_token = ctx.req.headers["device-token"];
                let filepath = files.upload_file.path;
                let dir = path.dirname(filepath)
                let basename = path.basename(filepath)
                let basename_pre = basename.split('.')[0];
                let basename_ext = basename.split('.')[1];

                let upload_path = "upload_bigmouth";
                let upload_file = path.join(upload_path, basename);
                Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-/api/upload_bigmouth], ' + upload_file);

                let output_path = path.join(dir, basename_pre + "_bigmouth." + basename_ext);
                let progress_path = path.join(dir, token + ".txt");
                let relative_path = path.join(upload_path, basename_pre + "_bigmouth." + basename_ext);

                let obj = {
                    progress:0,
                    path:relative_path,
                }
                let json = JSON.stringify(obj);
                fs.writeFile(progress_path, json, e => {
                    if (!e) {
                        let cmd = BIG_MOUTH + ' --input_path ' + filepath + ' --progress_path ' + progress_path
                        let child = exec.exec(cmd, function(error, stdout, stderr) {
                            if(error) {
                                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_bigmouth], ' + upload_file + ', ' + error.toString());
                                fs.unlink(progress_path);
                            } else {
                                if( device_token != "" ) {
                                    apns.send([device_token], "Congratulation! You have a new bigmouth video.");
                                }
                            }
                        });
                        Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-/api/upload_realsr], ' + cmd + ", pid:" +  child.pid);
                    } else {
                        Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_bigmouth], ' + upload_file + ', ' + e.toString());
                    }
                    resolve();
                })

            } catch (e) {
                respond.errno = 1;
                respond.errmsg = "Process fail";
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_bigmouth], ' + upload_file + ', ' + e.toString());
                resolve();

            } finally {
                // resolve();
            }
        })
    })

    let end = process.uptime() * 1000;
    respond.time = end - start + 'ms';

    ctx.body = respond;
});

proxyRouter.all('/api/query_bigmouth', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {
            path: "",
            progress:0
        }
    }

    let params = querystring.parse(ctx.querystring);
    let token = "";
    if (!Common.isNull(params.token)) {
        token = params.token;
    }

    if (token.length > 0) {
        let dir = path.join(__dirname, "../../static/upload_bigmouth");
        let progress_path = path.join(dir, token + ".txt");
        await new Promise(function(resolve, reject) {
            fs.readFile(progress_path, 'utf8', (err, data) => {
                if (!err) {
                    let obj = JSON.parse(data);
                    respond.data = obj;
                } else {
                    Common.log('http', 'warn', '[' + ctx.session.sessionId + ']-query_bigmouth], ' + token + ', ' + err);
                    respond.errno = -1;
                    respond.errmsg = 'No such file.';
                }
                resolve();
            });
        });
    }
    ctx.body = respond;
});

function shuffle(arr) {
    for (let i = arr.length; i; i--){
        let j = Math.floor(Math.random() * i);
        [arr[i - 1], arr[j]] = [arr[j], arr[i - 1]];
    }
    return arr;
}

function readDirRndImageSync(path, httpPath, size){
    let json = [];
    let pa = shuffle(fs.readdirSync(path)).slice(-size);
    pa.forEach(function(file, index){
        let info = fs.statSync(path + "/" + file)
        if( info.isFile() ){
            let absolutePath = path + "/" + file;
            let relativePath = httpPath + "/" + file;
            // console.log("absolutePath: ". absolutePath, ", relativePath: ", relativePath);

            let rex = /.*(.jpg|.jpeg|.png)/;
            let bFlag = rex.test(relativePath.toLowerCase());
            if ( bFlag ) {
                json.push(relativePath);
            }
        }
    })
    return json;
}

proxyRouter.all('/gallery', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {
            datalist:[]
        }
    }


    let categories = listDir(Common.AppGlobalVar.rootPath + "/static/facetoon/gallery");
    let categories_size = categories.length;
    let categories_real_size = categories_size;

    let category_item_count = Math.ceil(24 / categories_size);
    let use_categories = [];
    for(i = 0; i < categories_size; i++) {
        let item = categories[i];
        let items = readDirRndImageSync(Common.AppGlobalVar.rootPath + "/static/facetoon/gallery/" + item, "facetoon/gallery/" + item, category_item_count);
        if (items.length > 0) {
            use_categories.push(true);
        } else {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/gallery], ' + item + ' is empty.');
            use_categories.push(false);
            categories_real_size--;
        }
    }

    category_item_count = Math.ceil(24 / categories_real_size);
    Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-/gallery], page_item_count:' + category_item_count);
    for(i = 0; i < categories_size; i++) {
        if (use_categories[i]) {
            let item = categories[i];
            let items = readDirRndImageSync(Common.AppGlobalVar.rootPath + "/static/facetoon/gallery/" + item, "facetoon/gallery/" + item, category_item_count);
            respond.data.datalist = respond.data.datalist.concat(items);
        }
    }

    respond.data.datasize = respond.data.datalist.length;
    respond.data.datalist = shuffle(respond.data.datalist);

    // let asiame = readDirRndImageSync(Common.AppGlobalVar.rootPath + "/static/gallery_files/asiame", "gallery_files/asiame", 4);
    // let charmdate = readDirRndImageSync(Common.AppGlobalVar.rootPath + "/static/gallery_files/charmdate", "gallery_files/charmdate", 4);
    // let cathrynli = readDirRndImageSync(Common.AppGlobalVar.rootPath + "/static/gallery_files/cathrynli", "gallery_files/cathrynli", 4);
    // let artist = readDirRndImageSync(Common.AppGlobalVar.rootPath + "/static/gallery_files/artist", "gallery_files/artist", 12);

    // respond.data.datalist = shuffle(asiame.concat(charmdate, cathrynli, artist));
    ctx.body = respond;
});

function readDirSyncSortByDate(path, httpPath, page, page_size){
    page_size=page_size||12
    let json = [];
    let pa = fs.readdirSync(path)
        .map(function(v) {
            return {
                name:v,
                time:fs.statSync(path + "/" + v).mtime.getTime()
            };
        })
        .sort(function(a, b) { return b.time - a.time; })
        .map(function(v) { return v.name; })
        .slice((page - 1) * page_size, (page) * page_size);

    pa.forEach(function(file, index){
        let info = fs.statSync(path + "/" + file)
        if( info.isFile() ){
            let absolutePath = path + "/" + file;
            let relativePath = httpPath + "/" + file;
            // console.log("absolutePath: ". absolutePath, ", relativePath: ", relativePath);

            let rex = /.*(.jpg|.jpeg|.png|.mp4|.mov)/;
            let bFlag = rex.test(relativePath.toLowerCase());
            if ( bFlag ) {
                json.push(relativePath);
            }
        }
    })
    return json;
}

function listDir(path){
    let json = [];
    let pa = fs.readdirSync(path)
        .map(function(v) {
            return {
                name:v,
                time:fs.statSync(path + "/" + v).mtime.getTime()
            };
        })
        .sort(function(a, b) { return b.time - a.time; })
        .map(function(v) { return v.name; });

    pa.forEach(function(file, index){
        let info = fs.statSync(path + "/" + file);
        if( info.isDirectory() ) {
            let absolutePath = path + "/" + file;
            let rex = /^[^\.]/;
            let bFlag = rex.test(file.toLowerCase());
            // console.log("file:", file, "bFlag:", bFlag);
            if ( bFlag ) {
                json.push(file);
            }
        }
    })
    return json;
}

proxyRouter.all('/discovery', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {
            datalist:[]
        }
    }

    params = querystring.parse(ctx.querystring);
    page = 1;
    if (!Common.isNull(params.page)) {
        page = params.page;
    }
    page_size = 24;
    if (!Common.isNull(params.page_size)) {
        page_size = params.page_size;
    }

    let discovery = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/upload_discovery", "upload_discovery", page, page_size);
    respond.data.datalist = discovery;
    ctx.body = respond;
});

proxyRouter.all('/share_discovery', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {
        }
    }

    let start = process.uptime() * 1000;

    var form = new formidable.IncomingForm();
    form.encoding = 'utf-8';
    form.uploadDir = path.join(__dirname, "../../static/upload_discovery");
    form.keepExtensions = true;
    form.maxFieldsSize = 2 * 1024 * 1024;

    fs.mkdir(form.uploadDir, { recursive: true }, (err) => {
        if (err) {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-share_discovery], err:' + err);
        }
    });

    await new Promise(function(resolve, reject) {
        form.parse(ctx.req, function (err, fields, files) {
            resolve();
        });
    });
    let end = process.uptime() * 1000;
    respond.time = end - start + 'ms';

    ctx.body = respond;
});

proxyRouter.all('/api/wav2lip_list', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {
            datalist:[]
        }
    }

    await new Promise(function(resolve, reject) {
        let relativePath = Common.AppGlobalVar.rootPath + '/api/wav2lip/wav2lip_list.json';
        fs.readFile(relativePath, 'utf8', function (err, filedata) {
            if (err) {
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/wav2lip_list], ' + err);
                respond.errmsg = err;
                respond.errno = 1;
            } else {
                let fileobj = JSON.parse(filedata);
                respond.data.datalist = JSON.parse(filedata);
            }
            resolve();
        });
    });

    ctx.body = respond;
});

proxyRouter.all('/api/maser/discovery_category', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {
            datalist:[]
        }
    }

    let categories = listDir(Common.AppGlobalVar.rootPath + "/static/maser/gallery");
    respond.data.datalist = categories;
    ctx.body = respond;
});

proxyRouter.all('/api/maser/discovery', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {
            datalist:[]
        }
    }

    params = querystring.parse(ctx.querystring);
    page = 1;
    if (!Common.isNull(params.page)) {
        page = params.page;
    }
    page_size = 24;
    if (!Common.isNull(params.page_size)) {
        page_size = params.page_size;
    }
    category = '';
    if (!Common.isNull(params.category)) {
        category = params.category;
    }

    if (category.length == 0) {
        let categories = listDir(Common.AppGlobalVar.rootPath + "/static/maser/gallery");
        let categories_size = categories.length;
        let categories_real_size = categories_size;

        let category_item_count = Math.ceil(page_size / categories_size);
        let use_categories = [];
        for(i = 0; i < categories_size; i++) {
            let item = categories[i];
            let items = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/maser/gallery/" + item, "maser/gallery/" + item, page, category_item_count);
            if (items.length > 0) {
                use_categories.push(true);
            } else {
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/maser/discovery], ' + item + ' is empty.');
                use_categories.push(false);
                categories_real_size--;
            }
        }

        category_item_count = Math.ceil(page_size / categories_real_size);
        Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-/api/maser/discovery], page_item_count:' + category_item_count);
        for(i = 0; i < categories_size; i++) {
            if (use_categories[i]) {
                let item = categories[i];
                let items = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/maser/gallery/" + item, "maser/gallery/" + item, page, category_item_count);
                respond.data.datalist = respond.data.datalist.concat(items);
            }
        }
    } else {
        let item = category;
        let items = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/maser/gallery/" + item, "maser/gallery/" + item, page, page_size);
        respond.data.datalist = respond.data.datalist.concat(items);
    }
    respond.data.datasize = respond.data.datalist.length;
    respond.data.datalist = shuffle(respond.data.datalist);

    ctx.body = respond;
});

proxyRouter.all('/api/maser/dubnitskiy', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {
            datalist:[]
        }
    }

    params = querystring.parse(ctx.querystring);
    page = 1;
    if (!Common.isNull(params.page)) {
        page = params.page;
    }
    page_size = 24;
    if (!Common.isNull(params.page_size)) {
        page_size = params.page_size;
    }

    let items = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/maser/dubnitskiy/image", "maser/dubnitskiy/image", page, page_size);
    respond.data.datalist = items;

    ctx.body = respond;
});

proxyRouter.all('/api/gallery_cd', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {
            datalist:[]
        }
    }

    params = querystring.parse(ctx.querystring);
    page = 1;
    if (!Common.isNull(params.page)) {
        page = params.page;
    }
    page_size = 12;
    if (!Common.isNull(params.page_size)) {
        page_size = params.page_size;
    }

    let datalist = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/gallery_files/charmdate", "gallery_files/charmdate", page, page_size);
    respond.data.datalist = datalist;
    ctx.body = respond;
});

proxyRouter.all('/api/gallery_ame', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {
            datalist:[]
        }
    }

    params = querystring.parse(ctx.querystring);
    page = 1;
    if (!Common.isNull(params.page)) {
        page = params.page;
    }
    page_size = 12;
    if (!Common.isNull(params.page_size)) {
        page_size = params.page_size;
    }

    let datalist = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/gallery_files/asiame", "gallery_files/asiame", page, page_size);
    respond.data.datalist = datalist;
    ctx.body = respond;
});

proxyRouter.all('/api/gallery_artist', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {
            datalist:[]
        }
    }

    params = querystring.parse(ctx.querystring);
    page = 1;
    if (!Common.isNull(params.page)) {
        page = params.page;
    }
    page_size = 12;
    if (!Common.isNull(params.page_size)) {
        page_size = params.page_size;
    }

    let datalist = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/gallery_files/artist", "gallery_files/artist", page, page_size);
    respond.data.datalist = datalist;
    ctx.body = respond;
});

proxyRouter.all('/api/maser/proxy', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {
            datalist:[]
        }
    }

    await new Promise(function(resolve, reject) {
        let relativePath = Common.AppGlobalVar.rootPath + '/static/maser/proxy.json';
        fs.readFile(relativePath, 'utf8', function (err, filedata) {
            if (err) {
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/maser/proxy], ' + err);
                respond.errmsg = err;
                respond.errno = 1;
            } else {
                let fileobj = JSON.parse(filedata);
                respond.data.datalist = JSON.parse(filedata);
            }
            resolve();
        });
    });

    ctx.body = respond;
});

proxyRouter.all('/gc', async (ctx, next) => {
    let respond = {
        "errno":0,
        "errmsg":"",
        "userId":ctx.session.sessionId,
    }

    let start = process.uptime() * 1000;
    gc();
    // const Heapdump = require('heapdump');
    // Heapdump.writeSnapshot('./heapsnapshot/heapsnapshot-' + Date.now());
    let end = process.uptime() * 1000;
    respond.time = end - start + 'ms';

    ctx.body = respond;
});

proxyRouter.all('/api/update_phone_info', async (ctx, next) => {
    let respond = {
        "errno":0,
        "errmsg":"",
        "userId":ctx.session.sessionId,
    }
    obj = JSON.parse(ctx.request.rawBody.toLowerCase());

    dir = path.join(__dirname, "../../api/phone_info");
    fs.mkdir(dir, { recursive: true }, (e) => {
        if (e) {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/update_phone_info], e:' + e);
        }
    });

    if (!Common.isNull(obj.device_token) && obj.device_token.length > 0) {
        fname = path.join(dir, obj.device_token + ".json");
        Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-/api/update_phone_info], ' + fname);
        fs.writeFile(fname, ctx.request.rawBody, e => {
            if (e) {
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/update_phone_info], e:' + e);
            }
        });
    }

    ctx.body = respond;
});

proxyRouter.get('/', async (ctx, next) => {
    ctx.status = 302;
    ctx.redirect('/h5games/index.html');
});

module.exports = proxyRouter;