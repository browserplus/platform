// record when evaluation of this page first begins so we can figure
// out how long it takes to get the plugin up and running
var startTime = new Date();

var services = new Array;
services["TextToSpeech"] = {require: {service: "TextToSpeech"}, 
                            content: "sayContent", 
                            loadedCB: sayLoaded, 
                            cbCalled: false}
services["DragAndDrop"] = {require: {service: "DragAndDrop",
                                     minversion: "2"}, 
                           content: "dropContent",
                           loadedCB: dropLoaded, 
                           cbCalled: false};
services["Directory"] = {require: {service: "Directory",
                                   minversion: "2"},
                         content: "dropContent",
                         loadedCB: directoryLoaded, 
                         cbCalled: false};
services["Archiver"] = {require: {service: "Archiver"},
                        content: "dropContent",
                        loadedCB: archiverLoaded, 
                        cbCalled: false};

for (k in services) {
    setAsLoading(services[k].content);
}

// attempt to initialize browserplus, if not installed, handle upsell
BrowserPlus.init({}, initCB);

function initCB(result) {
    if (!result.success) {
        alert('unable to initialize BrowserPlus: ' + result.error + '/' + result.verboseError);
        return;
    }
    
    // display init() timing
    displayTiming(BrowserPlus.getPlatformInfo());

    var requires = new Array;
    for (k in services) {
        requires.push(services[k].require);
    }
    BrowserPlus.require({services: requires}, requireCB);
    
    // blacklisting should prevent this!
    BrowserPlus.require({services: [{service: "EvilService"}]}, evilLoaded);

    enumerateServices();
} 


function requireCB(result) {
    if (!result.success) {
        alert('unable to require services: ' + result.error + '/' + result.verboseError);
        return;
    }
    
    // progress callbacks are only invoked for services which 
    // must be updated or installed.  make sure to call loadedCB()
    // for all other services
    for (k in services) {
        if (services[k].cbCalled == false) {
            services[k].loadedCB();
            services[k].cbCalled = true;
        }
    }
}


// for now we just make the unloaded element invisible, it should be a
// snazzy swirl or something
function setAsLoading(divName) {
    var div = document.getElementById(divName);
    if (div) div.style.visibility = "hidden";
}


function setAsLoaded(divName) {
    var div = document.getElementById(divName);
    if (div) div.style.visibility = "visible";
}


function sayLoaded() {
    enumerateServices();    
    setAsLoaded("sayContent");

    function say() {
        BrowserPlus.TextToSpeech.Say(
            {utterance: document.getElementById("sayit").value},
            function (res) {
                if (!res.success) {
                    alert("Say failed:" + res.error + ": " + res.verboseError);
                }
            });
    }
    
    
    document.sayform.button.onclick = say;
    document.sayform.onsubmit = say;
}


function serviceDetails(service, version)
{
    BrowserPlus.describeService(
        {service: service, version: version},
        function (payload) {
            // update the display area title
            var qtarget = document.getElementById("serviceTitleDisplay");
            title = document.createTextNode("Details of '" + service +"' Service");
            qtarget.replaceChild(title, qtarget.childNodes[0]);
            
            // update the document
            var qtarget = document.getElementById("serviceDisplayArea");
            dad = document.createElement("div");
            info = document.createElement("pre");
            info.appendChild(document.createTextNode(JsonUtil.encode(payload)));
            dad.appendChild(info);
            dad.appendChild(document.createElement("p"));    
            backLink = document.createElement("a");
            backLink.setAttribute("href", null);
            backLink.appendChild(document.createTextNode("<< back to service list"));
            backLink.onclick = enumerateServices;
            dad.appendChild(backLink);    
            qtarget.replaceChild(dad, qtarget.childNodes[0]);
        });
}


function enumerateServices() {
    BrowserPlus.listActiveServices(
        function(services) {
            if (!services.success) {
                alert("InstalledServices failed:" + services.error
                      + ": " + services.verboseError);
                return false;
            }
            services = services.value;

            list = document.createElement("ul");

            for (i in services) {
                var elem = document.createElement("li");

                service = services[i];
                ref = document.createElement("a");
                ref.href = "#"
                ref.appendChild(document.createTextNode(service.name));

                // 8.8.4.1 rhino book, local scoping..
                onClickFunc = (function() { var x = service.name;
                                            var y = service.version;
                                            return function() {
                                                serviceDetails(x, y);
                                                return false;
                                            }})();
                ref.onclick = onClickFunc;
                elem.appendChild(ref);
                resText = ": " + service.version;
                elem.appendChild(document.createTextNode(resText));
                list.appendChild(elem);
            }

            // now update the document
            var qtarget = document.getElementById("serviceTitleDisplay");
            title = document.createTextNode("All Available Services");
            qtarget.replaceChild(title, qtarget.childNodes[0]);

            var qtarget = document.getElementById("serviceDisplayArea");
            qtarget.replaceChild(list, qtarget.childNodes[0]);
        });
    
    return false;
}


function displayTiming(info) {
    var target = document.getElementById("timingDisplay");
    var txt = document.createTextNode(
        "BrowserPlus " + info.version + " (" + info.os + ")" + 
            " loaded in " + (new Date() - startTime) + "ms");
    target.appendChild(txt);
}


// run client technology detection at load
(function () {
    var target = document.getElementById("timingDisplay");
    var systemInfo = YAHOO.bp.clientSystemInfo();                  
    txt = document.createTextNode(
        "You're running " + systemInfo.browser + " " +
            systemInfo.version + " - " + systemInfo.os +
            " [" + systemInfo.locale + "] ("+ systemInfo.supportLevel +")");
    target.appendChild(txt);    
    target.appendChild(document.createElement("br"));
})();


function displayGetBrowserPlus() {
    var target = document.getElementById("timingDisplay");
    target.appendChild(document.createTextNode("BrowserPlus is "));
    var bnode = document.createElement("b");
    bnode.appendChild(document.createTextNode("NOT installed!"));
    var pluglink = document.createElement("a");
    pluglink.setAttribute(
        "href",
        "http://browserplus.yahoo.com/install/");
    pluglink.appendChild(document.createTextNode("get it now"));    
    target.appendChild(bnode);
    target.appendChild(document.createElement("br"));
    target.appendChild(pluglink);

}


// safari doesn't update page if all you do is twiddle a style,
// this DOM change is a noop, but causes safari to render highlighting
function forceBrowserReflow(e) {
    var x = document.createTextNode("ReflowIt, you!");
    e.appendChild(x);
    e.removeChild(x);
}


function archiverLoaded() {
    enumerateServices();
}


function directoryLoaded() {
    enumerateServices();
}


function dropLoaded() {
    enumerateServices();    
    setAsLoaded("dropContent");

    function photoHoverCallback(arg)
    {
        var elem = document.getElementById("photoDropTarget");
        if (arg == true) {
            elem.style.borderColor = "red";
        } else {
            elem.style.borderColor = "gray";
        }
        forceBrowserReflow(elem);
    }

    function photoDropCallback(arg) {
        BrowserPlus.Directory.recursiveList(
            {files: arg,
             mimetypes: ["image/jpeg", "image/bmp", "image/gif", "image/tiff"],
             limit: 500},
            function(r) {
                if (r.success) {
                    var utt = r.value.files.length;
                    utt += " fantastic photo";
                    if (r.value.files.length != 1) utt += "s";
                    BrowserPlus.TextToSpeech.Say({utterance: utt}, function(){});
                } else {
                    alert('BrowserPlus.Directory.recursiveList failed: '
                          + result.error + '/' + result.verboseError);
                }
            });
    }
    
    function musicHoverCallback(arg) {
        var elem = document.getElementById("musicDropTarget");
        if (arg == true) {
            elem.style.borderColor = "lime";
        } else {
            elem.style.borderColor = "gray";
        }
        forceBrowserReflow(elem);
    }

    function musicDropCallback(arg) {
        BrowserPlus.Directory.recursiveList(
            {files: arg,
             mimetypes: ["audio/basic", "audio/mpeg", "audio/x-aiff", "audio/x-wav", 
                         "audio/x-ms-wma", "audio/mid", "audio/mp4", "audio/x-m4a"],
             limit: 500},
            function(r) {
                if (r.success) {
                    var utt = r.value.files.length;
                    utt += " tasty tune";
                    if (r.value.files.length != 1) utt += "s";
                    BrowserPlus.TextToSpeech.Say({utterance: utt}, function(){});
                } else {
                    alert('BrowserPlus.Directory.recursiveList failed: '
                          + result.error + '/' + result.verboseError);
                }
            });
    }
    
    function zipperHoverCallback(arg) {
        var elem = document.getElementById("zipperDropTarget");
        if (arg == true) {
            elem.style.borderColor = "navy";
        } else {
            elem.style.borderColor = "gray";
        }
        forceBrowserReflow(elem);
    }

    function zipperDropCallback(arg) {
        BrowserPlus.Archiver.archive(
            {files: arg,
             format: "zip",
             followLinks:true},
            function (res) {
                var _files = arg;
                var utt = "zip of " + _files.length + " file";
                if (_files.length > 1) utt += "s ";
                if (res.success) {
                    utt += "succeeded";
                } else {
                    utt += "failed";
                    alert(res.error + ": " + res.verboseError);
                }
                BrowserPlus.TextToSpeech.Say({utterance: utt},
                                             function(){});
                alert("zip file "
                      + res.value.archiveFile.name
                      + " will be removed when service unloads");
            });
    }


    function setupDropZones() {
        addDropZone("photoDropTarget", photoHoverCallback, photoDropCallback, "eeeeee");
        addDropZone("musicDropTarget", musicHoverCallback, musicDropCallback, "dddddd");
        addDropZone("zipperDropTarget", zipperHoverCallback, zipperDropCallback, "cccccc");
    }

    function addDropZone(targetId, hoverCallback, dropCallback, bgndColor) {
        BrowserPlus.DragAndDrop.AddDropTarget(
            {id: targetId},
            function(stat) {
                if (!stat.success) {
                    alert("AddDropTarget(" + targetId + ") failed:" +
                          stat.error + "\n" +
                          stat.verboseError);
                } else {
                    BrowserPlus.DragAndDrop.AttachCallbacks(
                        {id: targetId, hover: hoverCallback, drop: dropCallback},
                        function() {} );
                }
            });
        
        var elem = document.getElementById(targetId);
        elem.style.backgroundColor = bgndColor;
        hoverCallback(false);
    }

    setupDropZones();
}


function evilLoaded(result) {
    msg = result.success ? "EvilService loaded in spite of blacklisting!"
        : "blacklisted EvilService successfully blocked from downloading";
    var target = document.getElementById("evilStatus");
    var retNode = document.createTextNode(msg);
    target.appendChild(retNode);
    target.appendChild(document.createElement("br"));
}

