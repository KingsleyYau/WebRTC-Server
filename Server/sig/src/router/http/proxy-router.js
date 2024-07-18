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

const os = require('os')

const readline = require('readline');
const path = require('path');
const url = require('url');
const exec = require('child_process');
const formidable = require('formidable');
const mime = require('mime-types');
const querystring = require('querystring');
const util = require('util');
const moment = require("moment");

const fs = require('fs');
const afs = require('../../lib/fs-async.js')

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
    // 可以在这里增加hash filter, 减少缓存穿透
    await redisClient.client.multi().
    hgetall(
        'hash_user_online_' + user_id
    ).exec().then( (res) => {
        let all = JSON.stringify(res);
        Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-get], res:' + all);
        respond.res = res;
    }).catch( (err) => {
        Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-get], err:' + err);
        respond.errmsg = err;
    });

    ctx.body = respond;
});

proxyRouter.all('/nodes', async (ctx, next) => {
    let respond = {
        "errno":0,
        "errmsg":"",
        "res":-2,
        "userId":ctx.session.sessionId,
    }

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

    ctx.body = respond;
});

proxyRouter.all('/nodes_dbsize', async (ctx, next) => {
    let respond = {
        "errno":0,
        "errmsg":"",
        "res":-2,
        "userId":ctx.session.sessionId,
    }

    let nodes = redisClient.client.nodes('slave');
    await Promise.all(
        nodes.map(function (node) {
            return node.dbsize();
        })
    ).then( (res) => {
        respond.res = res;
    });

    ctx.body = respond;
});

proxyRouter.all('/rnd', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        res:-2,
        userId:ctx.session.sessionId,
    }

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
        Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-rnd], index:' + index + ', cursor:'+ cursor);
        await nodes[index].multi().
        scan(
            cursor, 'match', 'hash_user_online_*', 'count', 10
        ).exec().then( (res) => {
            let all = JSON.stringify(res);
            Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-rnd], res:' + all);
            respond.res = res;
        }).catch( (err) => {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-rnd], err:' + err);
            respond.errmsg = err;
        });
    }

    ctx.body = respond;
});

const P2C = AppConfig.python.pd + ' && python p2c_arg.py'
proxyRouter.all('/api/upload_toon', async (ctx, next) => {
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

    // ctx.session.data = new Array(1e7).join('*');
    let form = new formidable.IncomingForm();
    form.encoding = 'utf-8';
    form.uploadDir = path.join(__dirname + "../../../static/upload");
    form.keepExtensions = true;//保留后缀
    form.maxFieldsSize = 2 * 1024 * 1024;

    fs.mkdir(form.uploadDir, { recursive: true }, (err) => {
        if (err) {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_toon], err:' + err);
        }
    });

    let cartoon_dir = path.join(form.uploadDir, "cartoon");
    fs.mkdir(cartoon_dir, { recursive: true }, (err) => {
        if (err) {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_toon], err:' + err);
        }
    });

    await new Promise(function(resolve, reject) {
        form.parse(ctx.req, function (err, fields, files) {
            let cmd = ""
            let upload_file = "";
            try {
                let upload_file_path = files.upload_file.path;
                if (!Common.isNull(files.upload_file)) {
                    upload_file_path = files.upload_file.path;
                }
                let style_file_path = "";
                if (!Common.isNull(files.style_file)) {
                    style_file_path = files.style_file.path;
                }
                let dir = path.dirname(upload_file_path)
                let basename = path.basename(upload_file_path)
                let basename_pre = basename.split('.')[0];

                let align_face = 1;
                if( fields.align_face == "0" ) {
                    align_face = 0;
                }

                let keep_body = 0;
                if( fields.keep_body == "1" ) {
                    keep_body = 1;
                }

                let smooth = 0;
                if( fields.smooth == "1" ) {
                    smooth = 1;
                }

                let face_size = "default";
                if( fields.small_face == "1" ) {
                    face_size = "small";
                }

                let style = 0;
                if( !Common.isNull(fields.style) ) {
                    style = fields.style;
                }

                let toon_type = 'facetoon';
                if( !Common.isNull(fields.toon_type) ) {
                    toon_type = fields.toon_type;
                }

                let upload_path = "/upload/";
                upload_file = upload_path + basename;
                Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/upload_toon], ' + upload_file);

                let photo_path = path.join(dir, basename_pre + "_photo.jpg");
                let cartoon_path = path.join(dir, basename_pre + "_cartoon.jpg");

                cmd = P2C + ' --input_image ' + upload_file_path + ' --toon_type ' + toon_type + ' --style ' + style + " --align_face " + align_face + " --keep_body " + keep_body + " --smooth " + smooth + " --face_size " + face_size
                if (style == 4 && style_file_path.length > 0) {
                    cmd += ' --style_image ' + style_file_path
                }
                // exec.execSync(cmd)
                child = exec.exec(cmd, function(error, stdout, stderr) {
                    if (error) {
                        Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_toon], ' + upload_file + ', ' + error.toString());
                        respond.errno = 1;
                        respond.errmsg = error.message;
                    } else {
                        try {
                            if (fs.existsSync(photo_path)) {
                                let data = fs.readFileSync(photo_path);
                                data = new Buffer(data).toString('base64');
                                let photo_base64 = 'data:' + mime.lookup(photo_path) + ';base64,' + data;
                                respond.data.photo = photo_base64//upload_path + basename_pre + "_photo.png";

                                exec.exec('mv ' + photo_path  + ' ' + cartoon_dir)
                            }

                            data = fs.readFileSync(cartoon_path);
                            data = new Buffer(data).toString('base64');
                            let cartoon_base64 = 'data:' + mime.lookup(cartoon_path) + ';base64,' + data;

                            respond.data.cartoon = cartoon_base64//upload_path + basename_pre + "_cartoon.png";
                            respond.data.file_id = basename_pre.split('_')[1];

                            exec.exec('mv ' + cartoon_path  + ' ' + cartoon_dir)
                        } catch (e) {
                            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_toon], ' + upload_file + ', ' + e.toString());
                            respond.errno = 1;
                            respond.errmsg = e.message;
                        }
                    }
                    resolve();
                });
                Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/upload_toon], ' + cmd + ", pid:" +  child.pid);

            } catch (e) {
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_toon], ' + cmd + ', file:' + upload_file + ', ' + e.toString());
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

const SEG = AppConfig.python.pd + ' && python seg_arg.py'
proxyRouter.all('/api/upload_seg', async (ctx, next) => {
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

    // ctx.session.data = new Array(1e7).join('*');
    let form = new formidable.IncomingForm();
    form.encoding = 'utf-8';
    form.uploadDir = path.join(__dirname + "../../../static/upload_seg");
    form.keepExtensions = true;//保留后缀
    form.maxFieldsSize = 2 * 1024 * 1024;

    fs.mkdir(form.uploadDir, { recursive: true }, (err) => {
        if (err) {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_seg], err:' + err);
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

                parms = util.format('%j', fields);
                Common.log('http', 'notice', '[' + ctx.session.sessionId  + ']-/api/upload_seg], parms:' + parms);
                let crop_face = 1;
                if( fields.crop_face == "0" ) {
                    crop_face = 0;
                }

                let seg_face = 1;
                if( fields.seg_face == "0" ) {
                    seg_face = 0;
                }

                let seg_body = 0;
                if( fields.seg_body == "1" ) {
                    seg_body = 1;
                }

                let seg_detail_face = 0;
                if( fields.seg_detail_face == "1" ) {
                    seg_detail_face = 1;
                }

                let seg_detail_face_with_hair = 1;
                if( fields.seg_detail_face_with_hair == "0" ) {
                    seg_detail_face_with_hair = 0;
                }

                let align_face = 0;
                if( fields.align_face == "1" ) {
                    align_face = 1;
                }

                let keep_bg = 0;
                if( fields.keep_bg == "1" ) {
                    keep_bg = 1;
                }

                let fit_size = 0;
                if( fields.fit_size == "1" ) {
                    fit_size = 1;
                }

                let enhance_only = 0;
                if( fields.enhance_only == "1" ) {
                    enhance_only = 1;
                }

                let enhance_face_only = 0;
                if( fields.enhance_face_only == "1" ) {
                    enhance_face_only = 1;
                }

                let keep_body = 1;
                if( fields.keep_body == "0" ) {
                    keep_body = 0;
                }

                let smooth = 1;
                if( fields.smooth == "0" ) {
                    smooth = 0;
                }

                let face_size = "default";
                if( fields.small_face == "1" ) {
                    face_size = "small";
                }

                let tradio = 1.0;
                if( fields.tradio != "" ) {
                    tradio = Number(fields.tradio);
                }

                let upload_path = "/upload_seg/";
                let upload_file = upload_path + basename;
                Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/upload_seg], ' + upload_file);

                let photo_path = path.join(dir, basename_pre + "_photo.png");
                let cartoon_path = path.join(dir, basename_pre + "_seg.png");

                let cmd = SEG + ' --input_image ' + filepath + " --crop_face " + crop_face + " --seg_face " + seg_face + " --seg_body " + seg_body + " --seg_detail_face " + seg_detail_face + " --seg_detail_face_with_hair " + seg_detail_face_with_hair + " --align_face " + align_face + " --enhance_only " + enhance_only + " --keep_bg " + keep_bg + " --enhance_face_only " + enhance_face_only + " --fit_size " + fit_size + " --face_size " + face_size + " --keep_body " + keep_body + " --smooth " + smooth + " --tradio " + tradio
                // exec.execSync(cmd)
                Common.log('http', 'notice', '[' + ctx.session.sessionId  + ']-/api/upload_seg], ' + cmd);
                child = exec.exec(cmd, function(error, stdout, stderr) {
                    if(error) {
                        Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_seg], ' + upload_file + ', ' + error.toString());
                        respond.errno = 1;
                        respond.errmsg = error.message;
                    } else {
                        try {
                            let data = fs.readFileSync(cartoon_path);
                            data = new Buffer(data).toString('base64');
                            let cartoon_base64 = 'data:' + mime.lookup(cartoon_path) + ';base64,' + data;

                            respond.data.cartoon = cartoon_base64//upload_path + basename_pre + "_cartoon.png";
                            respond.data.file_id = basename_pre.split('_')[1];

                        } catch (e) {
                            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_seg], ' + upload_file + ', ' + e.toString());
                            respond.errno = 1;
                            respond.errmsg = e.message;
                        }
                    }
                    resolve();
                });
                Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/upload_seg], ' + cmd + ", pid:" +  child.pid);

            } catch (e) {
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_seg], ' + upload_file + ', ' + e.toString());
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

const PHOTO_TOOL = AppConfig.python.pd + ' && python photo_arg.py'
proxyRouter.all('/api/upload_photo', async (ctx, next) => {
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

    // ctx.session.data = new Array(1e7).join('*');
    let form = new formidable.IncomingForm();
    form.encoding = 'utf-8';
    form.uploadDir = path.join(__dirname + "../../../static/upload_photo");
    form.keepExtensions = true;//保留后缀
    form.maxFieldsSize = 2 * 1024 * 1024;

    fs.mkdir(form.uploadDir, { recursive: true }, (err) => {
        if (err) {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_photo], err:' + err);
        }
    });

    await new Promise(function(resolve, reject) {
        form.parse(ctx.req, function (err, fields, files) {
            upload_file = "";
            try {
                let dir = ""
                let basename = ""
                let basename_pre = ""
                let upload_path = "/upload_photo/"
                let upload_file = ""
                let photo_path = ""
                let cartoon_path = ""
                let filepath = ""
                if (!Common.isNull(files.upload_file)) {
                    filepath = files.upload_file.path;
                }

                let style = 'deoldfy';
                if( !Common.isNull(fields.style) ) {
                    style = fields.style;
                }

                let input_text = '""';
                if( !Common.isNull(fields.input_text) ) {
                    input_text = fields.input_text;
                }

                if (filepath.length > 0) {
                    dir = path.dirname(filepath)
                    basename = path.basename(filepath)
                    basename_pre = basename.split('.')[0];
                    upload_file = upload_path + basename;
                    photo_path = path.join(dir, basename_pre + "_photo.jpg");
                    cartoon_path = path.join(dir, basename_pre + "_result.jpg");
                    // Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/upload_photo], ' + upload_file);
                } else {
                    dir = form.uploadDir;
                    cartoon_path = path.join(dir, input_text + ".jpg");
                    // Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/upload_photo], ' + input_text);
                }

                let cmd = PHOTO_TOOL + " --style " + style
                if (filepath.length > 0) {
                    cmd += ' --input_image ' + filepath;
                }
                if (input_text.length > 0) {
                    cmd += ' --input_text ' + input_text;
                    cmd += ' --output_path ' + cartoon_path
                }
                // exec.execSync(cmd)
                child = exec.exec(cmd, function(error, stdout, stderr) {
                    if (error) {
                        Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_photo], ' + upload_file + ', ' + error.toString());
                        respond.errno = 1;
                        respond.errmsg = error.message;
                    } else {
                        try {
                            if (fs.existsSync(photo_path)) {
                                let data = fs.readFileSync(photo_path);
                                data = new Buffer(data).toString('base64');
                                let photo_base64 = 'data:' + mime.lookup(photo_path) + ';base64,' + data;
                                respond.data.photo = photo_base64;
                                if (style == 'photopen' || style == 'ernie_vilg') {
                                    fs.unlink(photo_path, (e) => {

                                    });
                                }
                            }

                            let data = fs.readFileSync(cartoon_path);
                            data = new Buffer(data).toString('base64');
                            let cartoon_base64 = 'data:' + mime.lookup(cartoon_path) + ';base64,' + data;
                            if (style == 'photopen' || style == 'ernie_vilg') {
                                fs.unlink(cartoon_path, (e) => {

                                });
                            }

                            respond.data.cartoon = cartoon_base64;
                            respond.data.file_id = basename_pre.split('_')[1];

                        } catch (e) {
                            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_photo], ' + upload_file + ', ' + e.toString());
                            respond.errno = 1;
                            respond.errmsg = e.message;
                        }
                    }
                    if (style == 'photopen' || style == 'ernie_vilg') {
                        fs.unlink(filepath, (e) => {

                        });
                    }

                    resolve();
                });
                Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/upload_photo], ' + cmd + ", pid:" +  child.pid);

            } catch (e) {
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_photo], ' + upload_file + ', ' + e.toString());
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

const CIYUN = AppConfig.python.pd + ' && python ciyun_arg.py'
proxyRouter.all('/api/upload_wordcloud', async (ctx, next) => {
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

    // ctx.session.data = new Array(1e7).join('*');
    let form = new formidable.IncomingForm();
    form.encoding = 'utf-8';
    form.uploadDir = path.join(__dirname + "../../../static/upload_wordcloud");
    form.keepExtensions = true;//保留后缀
    form.maxFieldsSize = 2 * 1024 * 1024;

    fs.mkdir(form.uploadDir, { recursive: true }, (err) => {
        if (err) {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_wordcloud], err:' + err);
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

                let upload_path = "/upload_wordcloud/";
                let upload_file = upload_path + basename;
                Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/upload_wordcloud], ' + upload_file);

                let cartoon_path = path.join(dir, basename_pre + "_ciyun.jpg");

                let cmd = CIYUN + ' --input_image ' + filepath
                // exec.execSync(cmd)
                child = exec.exec(cmd, function(error, stdout, stderr) {
                    if(error) {
                        Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_wordcloud], ' + upload_file + ', ' + error.toString());
                        respond.errno = 1;
                        respond.errmsg = error.message;
                    } else {
                        try {
                            let data = fs.readFileSync(cartoon_path);
                            data = new Buffer(data).toString('base64');
                            let cartoon_base64 = 'data:' + mime.lookup(cartoon_path) + ';base64,' + data;

                            respond.data.cartoon = cartoon_base64//upload_path + basename_pre + "_cartoon.png";
                            respond.data.file_id = basename_pre.split('_')[1];

                        } catch (e) {
                            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_wordcloud], ' + upload_file + ', ' + e.toString());
                            respond.errno = 1;
                            respond.errmsg = e.message;
                        }
                    }
                    resolve();
                });
                Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/upload_wordcloud], ' + cmd + ", pid:" +  child.pid);

            } catch (e) {
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/upload_wordcloud], ' + upload_file + ', ' + e.toString());
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
                let basename_ext = 'jpg';//basename.split('.')[1];

                Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/upload_realsr], ' + filepath);

                let upload_path = "/upload_realsr";
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
                        try {
                            child = exec.exec(cmd, function (error, stdout, stderr) {
                                if (error) {
                                    Common.log('http', 'warn', '[' + ctx.session.sessionId + ']-/api/upload_realsr], ' + upload_file + ', ' + error.toString());

                                } else {
                                    if (device_token != "") {
                                        apns.send([device_token], "Congratulation! You have a new supervision photo.");
                                    }
                                }
                            });
                            Common.log('http', 'debug', '[' + ctx.session.sessionId + ']-/api/upload_realsr], ' + cmd + ", pid:" + child.pid);
                        } catch (e) {
                            Common.log('http', 'debug', '[' + ctx.session.sessionId + ']-/api/upload_realsr], Fail, ' + cmd + ", pid:" + child.pid + ', e:' + e.toString());
                            fs.unlink(progress_path);
                        }
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

                let style = 1;
                if( !Common.isNull(fields.style)  ) {
                    style = fields.style;
                }

                let filepath = files.upload_file.path;
                let dir = path.dirname(filepath)
                let basename = path.basename(filepath)
                let basename_pre = basename.split('.')[0];
                let basename_ext = basename.split('.')[1];

                let upload_path = "upload_toon_video";
                let upload_file = path.join(upload_path, basename);
                Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/upload_toon_video], ' + upload_file);

                let progress_path = path.join(dir, token + ".txt");
                let relative_path = path.join(upload_path, basename_pre + "_cartoon." + basename_ext);

                let obj = {
                    progress:0,
                    path:relative_path,
                }

                let json = JSON.stringify(obj);
                fs.writeFile(progress_path, json, e => {
                    if (!e) {
                        let cmd = TOON_VIDEO + ' --input_path ' + filepath + ' --progress_path ' + progress_path + ' --style ' + style
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
                        Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/upload_toon_video], ' + cmd + ", pid:" +  child.pid);
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
                Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/upload_bigmouth], ' + upload_file);

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
                        Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/upload_bigmouth], ' + cmd + ", pid:" +  child.pid);
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
    Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/gallery], page_item_count:' + category_item_count);
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
            //console.log("absolutePath: ". absolutePath, ", relativePath: ", relativePath);

            let rex = /.*(.jpg|.jpeg|.png|.gif|.mp4|.mov)/;
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

    let discovery = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/upload_discovery", "/upload_discovery", page, page_size);
    respond.data.datalist = discovery;
    ctx.body = respond;
});

proxyRouter.all('/api/maser/share_discovery', async (ctx, next) => {
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
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/maser/share_discovery], err:' + err);
        }
    });

    await new Promise(function(resolve, reject) {
        form.parse(ctx.req, function (err, fields, files) {
            resolve();
        });
    });

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
            let items = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/maser/gallery/" + item, "/maser/gallery/" + item, page, category_item_count);
            if (items.length > 0) {
                use_categories.push(true);
            } else {
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/maser/discovery], ' + item + ' is empty.');
                use_categories.push(false);
                categories_real_size--;
            }
        }

        category_item_count = Math.ceil(page_size / categories_real_size);
        Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/maser/discovery], page_item_count:' + category_item_count);
        for(i = 0; i < categories_size; i++) {
            if (use_categories[i]) {
                let item = categories[i];
                let items = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/maser/gallery/" + item, "/maser/gallery/" + item, page, category_item_count);
                respond.data.datalist = respond.data.datalist.concat(items);
            }
        }
    } else {
        let item = category;
        let items = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/maser/gallery/" + item, "/maser/gallery/" + item, page, page_size);
        respond.data.datalist = respond.data.datalist.concat(items);
    }
    respond.data.datasize = respond.data.datalist.length;
    respond.data.datalist = shuffle(respond.data.datalist);

    ctx.body = respond;
});

proxyRouter.all('/api/maser/movie_category', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {
            datalist:[]
        }
    }

    let categories = listDir(Common.AppGlobalVar.rootPath + "/static/upload_movie");
    respond.data.datalist = categories;
    ctx.body = respond;
});

proxyRouter.all('/api/maser/movie', async (ctx, next) => {
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
        let categories = listDir(Common.AppGlobalVar.rootPath + "/static/upload_movie");
        let categories_size = categories.length;
        let categories_real_size = categories_size;

        let category_item_count = Math.ceil(page_size / categories_size);
        let use_categories = [];
        for(i = 0; i < categories_size; i++) {
            let item = categories[i];
            let items = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/upload_movie/" + item, "/upload_movie/" + item, page, category_item_count);
            if (items.length > 0) {
                use_categories.push(true);
            } else {
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/maser/movie], ' + item + ' is empty.');
                use_categories.push(false);
                categories_real_size--;
            }
        }

        category_item_count = Math.ceil(page_size / categories_real_size);
        Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/maser/movie], page_item_count:' + category_item_count);
        for(i = 0; i < categories_size; i++) {
            if (use_categories[i]) {
                let item = categories[i];
                let items = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/upload_movie/" + item, "/upload_movie/" + item, page, category_item_count);
                respond.data.datalist = respond.data.datalist.concat(items);
            }
        }
    } else {
        let item = category;
        let items = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/upload_movie/" + item, "/upload_movie/" + item, page, page_size);
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

    let items = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/maser/dubnitskiy/image", "/maser/dubnitskiy/image", page, page_size);
    respond.data.datalist = items;

    ctx.body = respond;
});

proxyRouter.all('/api/maser/diffusion', async (ctx, next) => {
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

    let items = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/maser/diffusion", "/maser/diffusion", page, page_size);
    respond.data.datalist = items;

    ctx.body = respond;
});

proxyRouter.all('/api/maser/camshare', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {
            datalist: []
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

    if (page == 1) {
        await new Promise(function (resolve, reject) {
            exec.exec('/root/Max/project/sync_camshare.sh', (err, stdout, stderr) => {
                if (err || stderr) {
                    respond.errno = -1;
                    respond.errmsg = stderr;
                }
                resolve();
            })
        });
    }

    let items = readDirSyncSortByDate(Common.AppGlobalVar.rootPath + "/static/maser/camshare", "/maser/camshare", page, page_size);
    respond.data.datalist = items;

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

    ctx.body = respond;
});

proxyRouter.all('/api/search_image', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
            imageUrl:"",
        }
    }

    // ctx.session.data = new Array(1e7).join('*');
    let form = new formidable.IncomingForm();
    form.encoding = 'utf-8';
    form.uploadDir = path.join(__dirname + "../../../static/upload_search");
    form.keepExtensions = true;//保留后缀
    form.maxFieldsSize = 2 * 1024 * 1024;

    fs.mkdir(form.uploadDir, { recursive: true }, (err) => {
        if (err) {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/search_image], err:' + err);
        }
    });

    await new Promise(function(resolve, reject) {
        form.parse(ctx.req, function (err, fields, files) {
            let filepath = files.upload_file.path;
            let dir = path.dirname(filepath)
            let basename = path.basename(filepath)
            let basename_pre = basename.split('.')[0];

            let upload_path = "/upload_search/";
            let upload_file = upload_path + basename;
            respond.data.imageUrl = upload_file;
            Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/search_image], ' + upload_file);
            resolve();
        })
    })

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
        Common.log('http', 'debug', '[' + ctx.session.sessionId  + ']-/api/update_phone_info], ' + fname);
        fs.writeFile(fname, ctx.request.rawBody, e => {
            if (e) {
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/update_phone_info], e:' + e);
            }
        });
    }

    ctx.body = respond;
});



const ROK_STOP = 'ps -ef | grep "rok.sh" | grep -v grep | awk \'{print $2}\' | xargs -I {} kill -9 {}\;'
// const ROK_TOOLS = '/Users/max/Documents/Project/Demo/python/pd/input/tmp/rok.sh '
const ROK_TOOLS = AppConfig.python.sh + 'rok.sh '
proxyRouter.all('/api/rank_queue', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
        }
    }
    Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-/api/rank_queue], body:' + ctx.request.rawBody);
    exec.execSync(ROK_STOP);
    await new Promise(function(resolve, reject) {
        exec.exec(ROK_STOP, (err, stdout, stderr) => {
            Common.log('http', 'info', '[' + ctx.session.sessionId + ']-/api/rank_queue], stdout:' + stdout);
            respond.data = stdout;
            resolve();
        });
    });
    // await new Promise(function(resolve, reject) {
    exec.exec(ROK_TOOLS + ' \'' + ctx.request.rawBody + '\'', (err, stdout, stderr) => {
        Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-/api/rank_queue], stdout:' + stdout);
        // respond.data = stdout;
        // resolve();
    });
    // });

    ctx.body = respond;
});

proxyRouter.all('/api/rank_queue_stop', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
        }
    }
    Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-/api/rank_queue_stop], body:' + ctx.request.rawBody.toLowerCase());
    exec.exec(ROK_STOP, (err, stdout, stderr) => {
        Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-/api/rank_queue_stop], stdout:' + stdout);
        respond.errmsg = stdout;
    });

    ctx.body = respond;
});

const ROK_DEAMON = AppConfig.python.rok
proxyRouter.all('/api/rok_title', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
        }
    }

    let line = ctx.querystring
    let params = querystring.parse(line);
    Common.log('http', 'notice', 'rok_title, line:' + line);

    let server = params.server;
    let x = params.x;
    let y = params.y;
    let title = params.title;

    let ip = ctx.req.headers["x-orig-ip"];
    if (Common.isNull(ip)) {
        ip = ctx.request.ip;
    }

    let record = {
        server:server,
        x:x,
        y:y,
        title:title,
    }

    let record_path = path.join('/root/Max/project/rok/web/record.txt')
    let data
    try {
        data = fs.readFileSync(record_path, 'utf8');
        lines = data.split(os.EOL);
        let json = JSON.stringify(record);
        Common.log('http', 'notice', 'rok_title, json:' + json);
        let result = lines.some((line, index, array) => {
            if (line.search(json) != -1) {
                let item = JSON.parse(line)
                respond.errno = 1;
                respond.errmsg = "请勿重复提交, 上次提交时间 " + item.summit_time;
                return true;
            }
        })
        if (!result) {
            let item = {
                record:record,
                summit_time:moment().format("YYYY-MM-DD HH:mm:ss"),
                ip:ip,
            }

            let json = JSON.stringify(item);
            fs.appendFileSync(record_path, json + os.EOL, 'utf8')
        }
    } catch (e) {
        Common.log('http', 'warn', '[' + ctx.session.sessionId + ']-/api/rok_title], ' + e);
        let item = {
            record:record,
            summit_time:moment().format("YYYY-MM-DD HH:mm:ss")
        }

        let json = JSON.stringify(item);
        fs.appendFileSync(record_path, json + os.EOL, 'utf8')
    }

    ctx.body = respond;
});

proxyRouter.all('/api/rok_title_jump', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
        }
    }

    let line = ctx.querystring
    let params = querystring.parse(line);
    Common.log('http', 'notice', '[' + ctx.session.sessionId + ']-/api/rok_title_jump], line:' + line);

    let server = params.server;
    let x = params.x;
    let y = params.y;
    let title = params.title;

    let record = {
        server:server,
        x:x,
        y:y,
        title:title,
    }

    let record_path = path.join('/root/Max/project/rok/web/record.txt')
    let data
    try {
        data = fs.readFileSync(record_path, 'utf8');
        lines = data.split(os.EOL);
        let json = JSON.stringify(record);
        Common.log('http', 'notice', '[' + ctx.session.sessionId + ']-/api/rok_title_jump], json:' + json);
        let result = lines.some((line, index, array) => {
            if (line.search(json) != -1) {
                let item = JSON.parse(line)
                respond.errno = 1;
                respond.errmsg = "请勿重复提交, 上次提交时间 " + item.summit_time;
                return true;
            }
        })
        if (!result) {
            let item = {
                record:record,
                summit_time:moment().format("YYYY-MM-DD HH:mm:ss")
            }

            let json = JSON.stringify(item);
            fs.writeFileSync(record_path, json + os.EOL + data, 'utf8')
            ret = exec.execSync(ROK_DEAMON);
            Common.log('http', 'notice', '[' + ctx.session.sessionId + ']-/api/rok_title_jump], ret:' + ret);
        }
    } catch (e) {
        Common.log('http', 'warn', '[' + ctx.session.sessionId + ']-/api/rok_title_jump], ' + e);
        let item = {
            record:record,
            summit_time:moment().format("YYYY-MM-DD HH:mm:ss")
        }

        let json = JSON.stringify(item);
        fs.writeFileSync(record_path, json + os.EOL + data, 'utf8')
        exec.execSync(ROK_DEAMON);
    }

    ctx.body = respond;
});

proxyRouter.all('/api/rok_title_queue', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
            size:0,
            queue:[]
        }
    }

    Common.log('http', 'notice', 'rok_title_queue');

    let record_path = path.join('/root/Max/project/rok/web/record.txt')
    let record_last_path = path.join('/root/Max/project/rok/web/record_last.txt')
    // let data = fs.readFileSync(record_path, 'utf8');

    Common.log('http', 'notice', 'rok_title_queue read ' + record_last_path);
    data1 = await afs.readFile(record_last_path, 'utf8')
    if (data1.length > 0) {
        let lines = data1.split(os.EOL);
        if (lines.length > 0) {
            let line = lines[0]
            let el = JSON.parse(line)
            respond.data.last = el
        }
    }

    Common.log('http', 'notice', 'rok_title_queue read ' + record_path);
    data = await afs.readFile(record_path, 'utf8')
    if (data.length > 0) {
        let lines = data.split(os.EOL);
        let queue = []
        lines.forEach((line) => {
            if (line.length > 0) {
                // Common.log('http', 'notice', 'rok_title_queue, line:' + line);
                let el = JSON.parse(line)
                queue.push(el)
            }
        });
        respond.data.size = queue.length;
        respond.data.queue = queue;
    }

    // let photo_path = path.join('/root/Max/project/rok/web/screencap.jpg')
    // Common.log('http', 'notice', 'rok_title_queue read ' + photo_path);
    // data = await afs.readFile(photo_path)
    // if (data.length > 0) {
    //     data = new Buffer(data).toString('base64');
    //     let photo_base64 = 'data:' + mime.lookup(photo_path) + ';base64,' + data;
    //     respond.data.photo = photo_base64
    // }

    ctx.body = respond;
});

proxyRouter.all('/api/rok_title_config', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
            config:{}
        }
    }

    Common.log('http', 'notice', 'rok_title_config');

    let title_config_path = path.join('/root/Max/project/rok/web/title_config.json')
    Common.log('http', 'notice', 'rok_title_config read ' + title_config_path);
    data = await afs.readFile(title_config_path, 'utf8')
    if (data.length > 0) {
        let el = JSON.parse(data)
        respond.data.config = el
    }

    ctx.body = respond;
});

proxyRouter.all('/api/rok_monitor', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
            monitor_count:{}
        }
    }

    Common.log('http', 'notice', 'rok_monitor');

    try {
        let monitor_file_path = path.join('/root/Max/project/rok/web/monitor_file.json')
        Common.log('http', 'notice', 'rok_monitor read ' + monitor_file_path);
        data = await afs.readFile(monitor_file_path, 'utf8')
        if (data.length > 0) {
            let el = JSON.parse(data)
            respond.data.monitor_count = el
        }
    } catch (e) {
        Common.log('http', 'warn', '[' + ctx.session.sessionId + ']-/api/rok_monitor], ' + e);
        respond.errno = 1;
        respond.errmsg = '当前没有开始监控'
    }

    ctx.body = respond;
});

proxyRouter.all('/api/rok_ranking_list', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
            ranking_date:"",
            ranking_list:{}
        }
    }

    Common.log('http', 'notice', 'rok_ranking_list');

    let params = querystring.parse(ctx.querystring);
    
    let date_string = params.date_string;
    if( Common.isNull(date_string) || date_string == '' ) {
        date_string = moment().format("YYYY-MM-DD");
    }
    respond.data.ranking_date = date_string;

    try {
        let file_path = path.join('/root/Max/project/rok/web/ranking/ranking_list_'+ date_string +'.json')
        Common.log('http', 'notice', 'rok_ranking_list read ' + file_path);
        data = await afs.readFile(file_path, 'utf8')
        if (data.length > 0) {
            let el = JSON.parse(data)
            respond.data.ranking_list = el;
        }
    } catch (e) {
        Common.log('http', 'warn', '[' + ctx.session.sessionId + ']-/api/rok_ranking_list], ' + e);
        respond.errno = 1;
        respond.errmsg = '没有该天统计'
    }

    ctx.body = respond;
});

const ROK_BOT = AppConfig.python.rok_api + ' && nohup python -u main.py --api true '
proxyRouter.all('/api/rok_bot', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
        }
    }

    let line = ctx.querystring
    let params = querystring.parse(line);
    Common.log('http', 'notice', '[' + ctx.session.sessionId + ']-/api/rok_bot], line:' + line);

    let device_name = params.device_name;
    let run = 0;
    if( params.run == "1" ) {
        run = 1;
    }

    let run_type = 'request_bot';
    if( params.run_type != "" ) {
        run_type = params.run_type;
    }

    let record = {
        running:run,
        name:device_name,
    }

    let record_path = path.join('/root/Max/project/rok/run/' + device_name + '.json')
    let log_path = path.join('/root/Max/project/rok/run/' + device_name + '.log')
    try {
        if (run) {
            // ret = exec.execSync(ROK_BOT + '--run ' + run + ' --device_name ' + device_name + ' > ' + log_path + ' 2>&1 &');
            data = fs.readFileSync(record_path, 'utf8');
            let old_record = JSON.parse(data);
            Common.log('http', 'info', '[' + ctx.session.sessionId  + ']-/api/rok_bot], ' + old_record.running);
            if (old_record.running) {
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/rok_bot], 正在打工, 不需要重复提交');
                respond.errno = 1;
                respond.errmsg = '正在打工, 不需要重复提交';
            } else {
                cmd = ROK_BOT + '--run ' + run + ' --device_name ' + device_name + ' --run_type '+ run_type + ' > ' + log_path + ' 2>&1 &'
                child = exec.exec(cmd, function (error, stdout, stderr) {
                    if (error) {
                        Common.log('http', 'warn', '[' + ctx.session.sessionId + ']-/api/rok_bot], ' + error.toString());
                        respond.errno = 1;
                        respond.errmsg = error.message;
                    }
                });
                Common.log('http', 'debug', '[' + ctx.session.sessionId + ']-/api/rok_bot], ' + cmd + ", pid:" + child.pid);
            }
        } else {
            let json = JSON.stringify(record);
            fs.writeFile(record_path, json + os.EOL, e => {
                if (e) {
                    Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/rok_bot], e:' + e);
                }
            });
        }
    } catch (e) {
        Common.log('http', 'warn', '[' + ctx.session.sessionId + ']-/api/rok_bot], ' + e);
    }

    ctx.body = respond;
});

proxyRouter.all('/api/rok_bot_snapshot', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
        }
    }

    let line = ctx.querystring
    let params = querystring.parse(line);
    Common.log('http', 'notice', '[' + ctx.session.sessionId + ']-/api/rok_bot_snapshot], line:' + line);

    let device_name = params.device_name;
    let photo_path = path.join('/root/Max/project/rok/run/' + device_name + '.jpg')
    try {
        if (fs.existsSync(photo_path)) {
            let data = fs.readFileSync(photo_path);
            data = new Buffer(data).toString('base64');
            let photo_base64 = 'data:' + mime.lookup(photo_path) + ';base64,' + data;
            respond.data.photo = photo_base64
        }
    } catch (e) {
        Common.log('http', 'warn', '[' + ctx.session.sessionId + ']-/api/rok_bot_snapshot], ' + e);
    }

    ctx.body = respond;
});

function readLastNLines(filePath, N, callback) {
    // 读取文件大小
    fs.stat(filePath, (err, stats) => {
        if(err) {
            return callback(err);
        }
        const fileSize = stats.size;
        // 创建可读流
        const readStream = fs.createReadStream(filePath, { start: fileSize - 1024, crlfDelay: Infinity });
        const rl = readline.createInterface({ input: readStream });
        const lines = [];

        // 逐行读取文件内容
        rl.on('line', (line) => {
            lines.push(line);
            if (lines.length < N) {
                lines.shift();
            }
        });

        // 读取完毕
        rl.on('close', () => {
            callback(null, lines);
        });
    });
}

proxyRouter.all('/api/rok_bot_log', async (ctx, next) => {
    let respond = {
        errno: 0,
        errmsg: "",
        userId: ctx.session.sessionId,
        data: {}
    }

    let line = ctx.querystring
    let params = querystring.parse(line);
    Common.log('http', 'notice', '[' + ctx.session.sessionId + ']-/api/rok_bot_log], line:' + line);

    let device_name = params.device_name;
    let log_path = path.join('/root/Max/project/rok/run/' + device_name + '.log')

    await new Promise(resolve => {
        try {
            cmd = 'tail -n 5 ' + log_path
            child = exec.exec(cmd, function (error, stdout, stderr) {
                if (error) {
                    Common.log('http', 'warn', '[' + ctx.session.sessionId + ']-/api/rok_bot_log], ' + error.toString());
                    respond.errno = 1;
                    respond.errmsg = error.message;
                } else {
                    respond.data.lines = stdout;
                }
                resolve()
            });
            Common.log('http', 'debug', '[' + ctx.session.sessionId + ']-/api/rok_bot], ' + cmd + ", pid:" + child.pid);
        } catch (e) {
            Common.log('http', 'warn', '[' + ctx.session.sessionId + ']-/api/rok_bot], ' + e);
        }
    })
    ctx.body = respond;
});

const ROK_GET_DEAD_INFO = AppConfig.python.rok_api + ' && nohup python -u main.py --run_type get_dead_info --api true --api_file '
proxyRouter.all('/api/rok_get_dead_info', async (ctx, next) => {
    let respond = {
        errno:0,
        errmsg:"",
        userId:ctx.session.sessionId,
        data:{
            dead_info:{}
        }
    }

    let line = ctx.querystring
    let params = querystring.parse(line);
    Common.log('http', 'notice', '[' + ctx.session.sessionId + ']-/api/rok_get_dead_info], line:' + line);

    let form = new formidable.IncomingForm();
    form.encoding = 'utf-8';
    form.uploadDir = path.join(__dirname + "../../../static/upload_rok");
    form.keepExtensions = true;//保留后缀
    form.maxFieldsSize = 2 * 1024 * 1024;

    fs.mkdir(form.uploadDir, { recursive: true }, (err) => {
        if (err) {
            Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/rok_get_dead_info], err:' + err);
        }
    });

    await new Promise(function(resolve, reject) {
        form.parse(ctx.req, function (err, fields, files) {
            try {
                let filepath = files.upload_file.path;
                cmd = ROK_GET_DEAD_INFO + filepath
                data = exec.execSync(cmd)
                Common.log('http', 'notice', '[' + ctx.session.sessionId + ']-/api/rok_get_dead_info], cmd:' + cmd + ', data:' + data);
                dead_info = JSON.parse(data)
                respond.data.dead_info = dead_info
                resolve()

            } catch (e) {
                Common.log('http', 'warn', '[' + ctx.session.sessionId  + ']-/api/rok_get_dead_info], ' + e.toString());
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

proxyRouter.get('/', async (ctx, next) => {
    ctx.status = 302;
    ctx.redirect('/maser/index.html');
});

module.exports = proxyRouter;