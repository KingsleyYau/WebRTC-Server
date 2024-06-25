const fs = require('fs');
const util = require('util');
/**
 * 将nodejs的fs模块的异步操作变为promise模式
 */
function FsAsync(){
    const prototype = this.constructor.prototype;
    const isFun = (e) => Object.prototype.toString.call(e) === '[object Function]';
    const isSync = (s) => s.indexOf('Sync') !== -1 || s.indexOf('sync') !== -1 ;
    for(let p in fs){
        const prop = fs[p];
        if(isFun(prop)){
            prototype[p] = isSync(prop.name) ? prop : util.promisify(prop);
        }
    }
}
module.exports = new FsAsync();