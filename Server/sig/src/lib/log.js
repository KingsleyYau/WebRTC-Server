// 日志管理类
log4js = require('log4js');
log4js.configure({
    appenders: {
        console: { type: 'console' },
        dateFile: { type: 'dateFile', filename: 'log/server', pattern: '-yyyy-MM-dd.log', alwaysIncludePattern: true },
    },
    categories: { default: { appenders: ['console', 'dateFile'], level: 'info' } },
    disableClustering:true
});

exports.getLogger = function(category) {
    return log4js.getLogger(category);
}
