// record when evaluation of this page first begins so we can figure
// out how long it takes to get the plugin up and running
var startTime = new Date();

var corelets = new Array;
corelets["TextToSpeech"] = {require: {service: "TextToSpeech"}, 
                            content: "sayContent", 
                            loadedCB: sayLoaded, 
                            cbCalled: false}
corelets["DesktopSearch"] = {require: {service: "DesktopSearch"}, 
                             content: "searchContent", 
                             loadedCB: searchLoaded, 
                             cbCalled: false};
corelets["YahooFinance"] = {require: {service: "YahooFinance",
                                      minversion: "1.0.6"}, 
                            content: "quoteContent", 
                            loadedCB: quoteLoaded,
                            cbCalled: false};
corelets["DragAndDrop"] = {require: {service: "DragAndDrop"}, 
                           content: "dropContent",
                           loadedCB: dropLoaded, 
                           cbCalled: false};
corelets["Zipper"] = {require: {service: "Zipper",
                                minversion: "2.0.0"},
                      content: "dropContent",
                      loadedCB: zipperLoaded, 
                      cbCalled: false};

for (k in corelets) {
    setAsLoading(corelets[k].content);
}

// attempt to initialize browserplus, if not installed, handle upsell
BPTool.Installer.show({}, initCB);

function initCB(result) {
    if (!result.success) {
        alert('unable to initialize BrowserPlus: ' + result.error + '/' + result.verboseError);
        return;
    }
    
    // display init() timing
    displayTiming(BrowserPlus.getPlatformInfo());

    var requires = new Array;
    for (k in corelets) {
        requires.push(corelets[k].require);
    }
    BrowserPlus.require({services: requires}, requireCB);
    
    // blacklisting should prevent this!
    BrowserPlus.require({services: [{service: "EvilCorelet"}]}, evilLoaded);

    enumerateCorelets();
} 

function requireCB(result) {
    if (!result.success) {
        alert('unable to require services: ' + result.error + '/' + result.verboseError);
        return;
    }
    
    // progress callbacks are only invoked for corelets which 
    // must be updated or installed.  make sure to call loadedCB()
    // for all other corelets
    for (k in corelets) {
        if (corelets[k].cbCalled == false) {
            corelets[k].loadedCB();
            corelets[k].cbCalled = true;
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
    enumerateCorelets();	
    setAsLoaded("sayContent");

    function say()
    {
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

function quoteLoaded() {
    setAsLoaded("quoteContent");
    enumerateCorelets();	

    function quote()
    {
	function quoteCallback(response)
	{
	    if (!response.success)
	    {
		alert("GetQuote failed:" + response.error
		      + ": " + response.verboseError);
		return false;
	    }
	    response = response.value;

	    // now update the document
	    var qtarget = document.getElementById("quotetarget");
	    var worked = document.createTextNode(
		response.name + " is at " + response.lastTrade);
	    qtarget.replaceChild(worked, qtarget.childNodes[0]);

	    // add an image for the fun of it
	    var imgtarg = document.getElementById("quoteimagetarget");
	    var imgelem = document.createElement("img");
	    imgelem.setAttribute("src",
				 "http://ichart.finance.yahoo.com/t?s=" + symbol);
	    imgtarg.appendChild(imgelem);

	    // Talk!
	    var updown = response.change < 0.0 ? "down" : "up";
	    var change =
		response.change > 0.0 ? response.change : response.change * -1;
	    var res = BrowserPlus.TextToSpeech.Say(
		{utterance: response.name + " is " + updown + " " + change},
                function (res) {
                    if (!res.success) {
                        alert("say failed: " + res.error + ": " + res.verboseError);
                    }
                });
        }

	// access symbol from DOM
	var symbol = document.getElementById("symbol").value;

	// fetch the quote using nbrowserplus
	BrowserPlus.YahooFinance.GetQuote({ symbol: symbol }, quoteCallback);

	// indicate that we're loading the quote
	var imgtarget = document.getElementById("quoteimagetarget");
	while (imgtarget.childNodes.length > 0) {
	    imgtarget.removeChild(imgtarget.firstChild);
	}

	var loading = document.createTextNode("loading quote for "+
					      symbol.toUpperCase() +"...");
	var qtarget = document.getElementById("quotetarget");
	qtarget.replaceChild(loading, qtarget.childNodes[0]);

	return false;
    }

    document.quoteform.button.onclick = quote;
    document.quoteform.onsubmit = quote;
}

function searchLoaded() { 
    enumerateCorelets();	
    setAsLoaded("searchContent");

    function search()
    {
	function searchComplete(rez)
	{
	    if (!rez.success) {
		alert("Search failed:" + rez.error + ": " + rez.verboseError);
		return false;
	    }
	    rez = rez.value;

	    BrowserPlus.TextToSpeech.Say(
		{utterance: rez.returnedResults + " results for " + rez.query},
		function(){});

	    // now update the document
	    var target = document.getElementById("searchtarget");
	    var done = document.createTextNode("searched ");
	    target.replaceChild(done, target.childNodes[0]);

	    // now update the document
	    var target = document.getElementById("searchtarget");

	    // update the result count
	    var countNode = target.childNodes[4];
	    var newCountNode = document.createTextNode(rez.totalResults);
	    target.replaceChild(newCountNode, countNode);
	}

	function searchResults(rez)
	{
	    var target = document.getElementById("searchtarget");

	    // update the current result count
	    var countNode = target.childNodes[2];
	    var newCount = Number(countNode.data) + rez.length
	    var newCountNode = document.createTextNode(newCount);
	    target.replaceChild(newCountNode, countNode);

	    // update the total result count
	    countNode = target.childNodes[4];
	    newCount = Number(countNode.data) + rez.length
	    newCountNode = document.createTextNode(newCount);
	    target.replaceChild(newCountNode, countNode);
	    
	    for (var i = 0; i < rez.length ; i++)
	    {
		response = rez[i];
		var a = document.createElement("a");
		a.appendChild(document.createTextNode(response.title));
		target.appendChild(a);
		var sz = document.createTextNode(" (" +
						 (response.size / 1000) + "k)");
		target.appendChild(sz);
		target.appendChild(document.createElement("br"));
	    }
	}

	// access symbol from DOM
	var query = document.getElementById("query").value;

	// start the search
	BrowserPlus.DesktopSearch.Search({ query: query,
					   type: "all",
					   callback: searchResults
					 },
					 searchComplete);

	// indicate that we're loading the quote
	var target = document.getElementById("searchtarget");
	while (target.childNodes.length > 0) {
	    target.removeChild(target.firstChild);
	}
	
	target.appendChild(document.createTextNode("searching "));
	target.appendChild(
	    document.createTextNode("\"" + query + "\".  displaying "));
	target.appendChild(document.createTextNode("0"));
	target.appendChild(document.createTextNode("/"));
	target.appendChild(document.createTextNode("0"));
	target.appendChild(document.createTextNode(" results"));
	target.appendChild(document.createElement("p"));

	return false;
    }
    document.searchform.button.onclick = search;
    document.searchform.onsubmit = search;
}

function coreletDetails(corelet, version)
{
    BrowserPlus.describeService(
        {service: corelet, version: version},
        function (payload) {
            // update the display area title
            var qtarget = document.getElementById("coreletTitleDisplay");
            title = document.createTextNode("Details of '" + corelet +"' Corelet");
            qtarget.replaceChild(title, qtarget.childNodes[0]);
            
            // update the document
            var qtarget = document.getElementById("coreletDisplayArea");
            dad = document.createElement("div");
            info = document.createElement("pre");
            info.appendChild(document.createTextNode(JsonUtil.encode(payload)));
            dad.appendChild(info);
            dad.appendChild(document.createElement("p"));    
            backLink = document.createElement("a");
            backLink.setAttribute("href", null);
            backLink.appendChild(document.createTextNode("<< back to corelet list"));
            backLink.onclick = enumerateCorelets;
            dad.appendChild(backLink);    
            qtarget.replaceChild(dad, qtarget.childNodes[0]);
        });
}

function enumerateCorelets()
{
    BrowserPlus.listActiveServices(
	function(corelets) {
            if (!corelets.success)
            {
                alert("InstalledCorelets failed:" + corelets.error
                      + ": " + corelets.verboseError);
                return false;
            }
            corelets = corelets.value;

            list = document.createElement("ul");

            for (i in corelets)
            {
                var elem = document.createElement("li");

                corelet = corelets[i];
                ref = document.createElement("a");
                ref.href = "#"
                ref.appendChild(document.createTextNode(corelet.name));

                // 8.8.4.1 rhino book, local scoping..
                onClickFunc = (function() { var x = corelet.name;
                                            var y = corelet.version;
                                            return function() {
                                                coreletDetails(x, y);
                                                return false;
                                            }})();
                ref.onclick = onClickFunc;
                elem.appendChild(ref);
                resText = ": " + corelet.version;
                elem.appendChild(document.createTextNode(resText));
                list.appendChild(elem);
            }

            // now update the document
            var qtarget = document.getElementById("coreletTitleDisplay");
            title = document.createTextNode("All Available Corelets");
            qtarget.replaceChild(title, qtarget.childNodes[0]);

            var qtarget = document.getElementById("coreletDisplayArea");
            qtarget.replaceChild(list, qtarget.childNodes[0]);
        });
    
    return false;
}


function displayTiming(info)
{
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
	    " [" + systemInfo.locale + "] ("+systemInfo.supportLevel+")");
    target.appendChild(txt);    
    target.appendChild(document.createElement("br"));
})();

function displayGetBrowserPlus()
{
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

function zipperLoaded() {
    enumerateCorelets();
}

function dropLoaded() {
    enumerateCorelets();	
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
	var utt = arg.length;
	utt += " fantastic photo";
        if (arg.length > 1) utt += "s";
        BrowserPlus.TextToSpeech.Say({utterance: utt}, function(){});

        logDrop(arg);
    }
    
    function musicHoverCallback(arg)
    {
        var elem = document.getElementById("musicDropTarget");
        if (arg == true) {
            elem.style.borderColor = "lime";
        } else {
            elem.style.borderColor = "gray";
        }
	forceBrowserReflow(elem);
    }

    function musicDropCallback(arg) {
        var utt = arg.actualSelection.length + " selection";
        if (arg.actualSelection.length > 1) utt += "s";
        utt += " resulted in " + arg.files.length + " tasty tune";
        if (arg.files.length > 1) utt += "s";
        BrowserPlus.TextToSpeech.Say({utterance: utt}, function(){});

        logDrop(arg);
    }
    
    function allFilesHoverCallback(arg)
    {
        var elem = document.getElementById("allFilesDropTarget");
        if (arg == true) {
            elem.style.borderColor = "navy";
        } else {
            elem.style.borderColor = "gray";
        }
	forceBrowserReflow(elem);
    }

    function allFilesDropCallback(arg) {
	var files = [];
	if (arg.actualSelection) {
	    files = arg.actualSelection;
	} else if (arg.length > 0) {
	    files = arg;
	}
	BrowserPlus.Zipper.zip({files: files, followLinks:true},
				     function (res) {
					 var utt = "zip of " + files.length + " file";
					 if (files.length > 1) utt += "s ";
					 if (res.success) {
					     utt += "succeeded";
					 } else {
					     utt += "failed";
					     alert(res.error + ": " + res.verboseError);
					 }
					 BrowserPlus.TextToSpeech.Say({utterance: utt},
								      function(){});
					 alert("zip file "
					       + res.value.zipFile.name
					       + " will be removed when corelet unloads");
				     });
	logDrop(arg);
    }

    function logDrop(arg)
    {
        var msg = "";
        if (arg.actualSelection) {
            msg = msg + "actualSelection: ";
            for (var i = 0; i < arg.actualSelection.length; i++) {
                msg = msg + arg.actualSelection[i].BrowserPlusHandleID + " = " 
                    + arg.actualSelection[i].BrowserPlusHandleName + ", ";
            }
            msg = msg + "\nfiles: ";
            for (var i = 0; i < arg.files.length; i++) {
                msg = msg + arg.files[i].handle.BrowserPlusHandleID
                    + " = " + arg.files[i].handle.BrowserPlusHandleName
                    + " (parent = " + arg.files[i].parent + ")\n";
            }
        } else {
            for (var i = 0; i < arg.length; i++) {
                msg = msg + arg[i].BrowserPlusHandleID + " = " 
                    + arg[i].BrowserPlusHandleName + ", ";
            }
        }
        //alert(msg);
    }
    
    function setupDropZones()
    {
        addDropZone("photoDropTarget", ["image/jpeg", "image/bmp", "image/gif", "image/tiff"],
                    false, photoHoverCallback, photoDropCallback, "eeeeee");
        addDropZone("musicDropTarget",
                    ["audio/basic", "audio/mpeg", "audio/x-aiff", "audio/x-wav", 
                     "audio/x-ms-wma", "audio/mid", "audio/mp4"],
                    true, musicHoverCallback, musicDropCallback, "dddddd");
        addDropZone("allFilesDropTarget", [],
                    true, allFilesHoverCallback, allFilesDropCallback, "cccccc");
    }

    function addDropZone(targetId, mimeTypes, includeGestureInfo,
                         hoverCallback, dropCallback, bgndColor)
    {
        BrowserPlus.DragAndDrop.AddDropTarget(
            {id: targetId, includeGestureInfo: includeGestureInfo, mimeTypes: mimeTypes},
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
    
    function enableDropZone(targetId, enable, bgndColor)
    {
        BrowserPlus.DragAndDrop.EnableDropTarget(
            {id: targetId, enable: enable}, 
            function(stat) {
                if (!stat.success) {
                    alert("EnableDropTarget(" + targetId + ") failed:" +
                          stat.error + "\n" +
                          stat.verboseError);
                } else {
                    var elem = document.getElementById(targetId);
                    if (enable) {
                        elem.style.backgroundColor = bgndColor;
                    } else {
                        elem.style.backgroundColor = "ffffff";
                    }
                    forceBrowserReflow(elem);
                }
            });
    }
    
    function onPhotosClick()
    {
        enableDropZone("photoDropTarget", document.dropform.button1.checked, "eeeeee");
    }
    
    function onMusicClick()
    {
        enableDropZone("musicDropTarget", document.dropform.button2.checked, "dddddd");
    }
    
    function onAllFilesClick()
    {
        enableDropZone("allFilesDropTarget", document.dropform.button3.checked, "cccccc");
    }

    setupDropZones();

    document.dropform.button1.onclick = onPhotosClick;
    document.dropform.button2.onclick = onMusicClick;
    document.dropform.button3.onclick = onAllFilesClick;
}

function evilLoaded(result) {
    msg = result.success ? "EvilCorelet loaded in spite of blacklisting!"
        : "blacklisted EvilCorelet successfully blocked from downloading";
    var target = document.getElementById("evilStatus");
    var retNode = document.createTextNode(msg);
    target.appendChild(retNode);
    target.appendChild(document.createElement("br"));
}

