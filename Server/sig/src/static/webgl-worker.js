let filter;

function worker_draw() {
    filter.draw();
    setTimeout(() => {
        worker_draw();
    }, 30);
}

self.onmessage = function(e) {
    console.log("worker::onmessage: ", e);
    switch (e.data.msg) {
        case 'draw':{
            self.importScripts('imagefilter.js');
            filter = new ImageFilter();
            filter.init(e.data.canvas);
            filter.loadImage("img/1.jpg");
            worker_draw();
        }break;
    }
};