
function pollProgress()
{
    setTimeout(function() {
        progress = BPInstaller.progress();
        document.getElementById("statusBarProgress").style.width = String(progress) + "%";	
        if (progress >= 100) {
            setPage("done");
        } else {
            pollProgress();
        }
    }, 100);
}

document.installaction.begin.onclick = function() {
    setPage("progress");
    pollProgress();
    BPInstaller.beginInstall();
};

document.installaction.cancel.onclick = function() {
    BPInstaller.cancelInstall();
};

document.goodbye.alldone.onclick = function() {
    BPInstaller.allDone();
};

/* change the page */
function setPage(name) {
    document.getElementById("splashPage").style.display = "none";
    document.getElementById("progressPage").style.display = "none";
    document.getElementById("donePage").style.display = "none";
    document.getElementById(name+"Page").style.display = "block";
}

/* Init function.  Sets language strings. */
(function() {
    var id;
    if (LANG) {
        for (key in LANG) {
            if (LANG.hasOwnProperty(key)) {
                id = document.getElementById(key);
                if (id) {
                    if (id.value) {
                        id.value = LANG[key];
                    } else {
                        id.innerHTML = LANG[key];
                    }
                }
            }
        }
    }

})();