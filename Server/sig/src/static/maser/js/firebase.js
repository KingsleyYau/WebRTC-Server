async function addScript(url) {
    return new Promise((resolve, reject) => {
        var script = document.createElement('script');
        script.setAttribute('type', 'text/javascript');
        script.setAttribute('src', url);
        document.getElementsByTagName('body')[0].appendChild(script);
        script.onload = function () {
            // do something
            resolve()
        }
    });
}

// Your web app's Firebase configuration
// For Firebase JS SDK v7.20.0 and later, measurementId is optional
var firebaseConfig = {
    apiKey: "AIzaSyBe7vpzJjqXaE7EkkFELIqaRdDo4PUQKuM",
    authDomain: "maxzoon-57c4c.firebaseapp.com",
    projectId: "maxzoon-57c4c",
    storageBucket: "maxzoon-57c4c.appspot.com",
    messagingSenderId: "579995832541",
    appId: "1:579995832541:web:61596f6d731f7685174b34",
    measurementId: "G-YKM74EB5KM"
};

async function initFirebase() {
    <!-- The core Firebase JS SDK is always required and must be listed first -->
    await addScript("https://www.gstatic.com/firebasejs/8.8.0/firebase-app.js")
    await addScript("https://www.gstatic.com/firebasejs/8.8.0/firebase-analytics.js")

    // Initialize Firebase
    firebase.initializeApp(firebaseConfig);
    firebase.analytics();
}

async function initBaidu() {
    await addScript("https://hm.baidu.com/hm.js?bb3e07df9ba2b5ceca9af193bc152f4e")
}
initFirebase()
initBaidu()