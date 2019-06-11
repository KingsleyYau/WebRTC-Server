const mysql = require('mysql');
const dbconfig = require('../config/db-config');

var pool = mysql.createPool({
    host     : dbconfig.db.host,
    user     : dbconfig.db.username,
    password : dbconfig.db.password,
    database : dbconfig.db.database,
    port     : dbconfig.db.port
});

var dbQuery = function(sql, options, cb) {
    pool.query(sql, options,function(err, results, fields) {
        // 输出日志
        if ( err ) {

        }
        cb(err, results, fields);
    });
};

module.exports = dbQuery;