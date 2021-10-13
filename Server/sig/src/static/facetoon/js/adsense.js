async function addScript(url) {
    return new Promise((resolve, reject) => {
        var script = document.createElement('script');
        script.setAttribute('type','text/javascript');
        script.setAttribute('src',url);
        script.setAttribute('data-ad-client','ca-pub-3883793041048280');
        document.getElementsByTagName('head')[0].appendChild(script);
        script.onload = function () {
            // do something
            resolve()
        }
    });
}

async function initAdSense() {
    addScript("https://pagead2.googlesyndication.com/pagead/js/adsbygoogle.js")
}
initAdSense()
