var behavior = function() {

// view manager
var ViewMan = (function() {

    var YD = YAHOO.util.Dom;
    var YE = YAHOO.util.Event;
    var PermMap;

    var EntryRemoveTmpl = 
        '<div class="entry">' + 
        '  <div class="entryName">{name}</div>' +
        '  <div class="entryRemove" title="{help}"></div>' +
        '  <div class="entryDesc">{desc}</div>' + 
        '</div>';

    var EntryTmpl = 
        '<div class="entry">' + 
        '  <div class="entryName">{name}</div>' +
        '  <div class="entryDesc">{desc}</div>' + 
        '</div>';

    var prettySize = function(s) {
        var rval, oneMeg = (1024*1024), oneK = 1024, suffix;
        if (s > oneMeg) {
            rval = s / oneMeg;
            suffix = "MB";
        } else {
            rval = s / oneK;
            suffix = "KB";
        }
        rval = rval.toString();
        rval = rval.slice(0, rval.indexOf('.') + 3);
        return rval + suffix;
    };
    
    var wbr = function(s) {
        return s.replace(/\//g, "/<wbr>");
    };

    var permsRemove = function(id)
    {
        var re = /permid_\d+/;
        
        var getVisiblePerms = function(el)
        {
            var cn = el.className;
            return re.test(cn) && el.style.display !== "none";
        };
        
        var p, args = [], els;
        if (id) {
            p = PermMap[id];
            args.push({site: p.site, permissionKey: p.permissionKey, value: "reset"});
        } else {
            // find rows that are displayed (unfiltered)            
            els = YD.getElementsBy(getVisiblePerms, "SPAN", "permsList");

            for (i = 0; i < els.length; i++) {
                p = PermMap[els[i].className];
                args.push({site: p.site, permissionKey: p.permissionKey, value: "reset"});
            }
        }

        BPState.set("permissions", args);

        // refresh view after removing permissions
        updateCurrentView();
    };

    YE.addListener("generalEnableButton", "click", 
        function(e) { 
            var wasEnabled = BPState.getSync("enabled");
            BPState.set("enabled", wasEnabled ? false : true);
            views.general.update();
        });

    YE.addListener("generalUninstallButton", "click", 
        function(e) {
            BPState.uninstall();
            views.general.update();
        });

    YE.addListener("generalAlwaysAskId", "change", 
        function(e) { 
            BPState.set("requireDomainApproval", YD.get("generalAlwaysAskId").checked);
        });

    YE.addListener("servicesList", "click",
        function(e) {
            var el = YE.getTarget(e), els, name, version, i;
            if (el.className === "entryRemove") {
                els = YD.getElementsBy(function(el) { return true;}, "SPAN", el.parentNode)

                for (i = 0; i < els.length; i++) {
                    if (els[i].className === "realname") {
                        name = els[i].innerHTML;
                    } else if (els[i].className === "version") {
                        version = els[i].innerHTML;
                    }
                }

                if (name && version) {
                    BPState.deleteService(name, version);
                    updateCurrentView();
                }
            }
        });

    YE.addListener("permsList", "click", 
        function(e) {
            var el = YE.getTarget(e), els, name, version, i;
            if (el.className === "entryRemove") {
                els = YD.getElementsBy(function(el) { return true;}, "SPAN", el.parentNode)

                for (i = 0; i < els.length; i++) {
                    permsRemove(els[i].className);
                }
            }
        });

    YE.addListener("troubleshootRestart", "click", 
        function(e) {
            BPState.restart();
        });

    YE.addListener("troubleshootSend", "click", 
        function(e) {
            var includeLogs = YD.get("troubleshootIncludeLogs").checked;
            var report = '"' + YD.get("troubleDescription").value + '"';
            YD.get("troubleshootSendStatus").innerHTML = "<%= config.troubleshootSending %>";
            var rv = BPState.sendReport(report, includeLogs);
            YD.get("troubleshootSendStatus").innerHTML = rv ? "<%= config.troubleshootSentYes %>" : "<%= config.troubleshootSentNo %>";
         });

    YE.addListener("permissionsRemoveAll", "click", function(e){permsRemove();});

    var views = {
        general: {
            divId: "generalPane",
            buttonId: "generalTab",
            updatePollMS: 3000,
            update: function() {
                
                var updatePanel = function(sites) {
                    // no sites comes back as undefined
                    if (typeof sites !== 'object') {
                        sites = []
                    }
                    var i, j, desc = "", str = "", s;

                    YD.get("numSites").innerHTML = sites.length;

                    for (i=0; i < sites.length; i++) {

                        desc = "<%= config.generalServicesUsed %> " + sites[i].services.length;

                        for (j = 0; j < sites[i].services.length; j++) {
                            s = sites[i].services[j]
                            desc += (j === 0 ? " - " : ", ") + s.service + "(" +s.version + ")";
                        }

                        str += YAHOO.lang.substitute(EntryTmpl, {name: sites[i].site, desc: desc});
                    }

                    YD.get("generalList").innerHTML = str;
                };

                var isEnabled = BPState.getSync("enabled");
                YD.get("generalEnableButton").innerHTML = isEnabled ? "<%= config.generalDisableButton %>" : "<%= config.generalEnableButton %>";
                YD.get("generalEnabled").innerHTML = isEnabled ? "<%= config.generalEnabled %>" : "<%= config.generalDisabled %>";
                BPState.get("requireDomainApproval", function(b) { YD.get("generalAlwaysAskId").checked = b; });

                // update version string
                var vers = BPState.getSync("version");
                
                YD.get("generalVersionText").innerHTML = (vers == "" ? "<%= config.generalNotInstalledText %>" : "<%= config.generalInstalledText %>");
                YD.get("generalVersion").innerHTML = vers;

                var ok = BPState.get("activeSites", function(sites) {
                    updatePanel(sites);
                });
                if (!ok) {
                    updatePanel([]);
                }
            }
        },

        services: {
            divId: "servicesPane",
            buttonId: "servicesTab",
            updatePollMS: 3000,
            update: function() {
                
                var updatePanel = function(services) {
                    // no services comes back as undefined
                     if (typeof services !== 'object') {
                         services = []
                     }

                    // update number
                    YD.get("numServices").innerHTML = services.length;

                    var str = "", i = 0;
                    var name;
                    //var s = YD.get("servicesList");

					BPState.log(services);

                    for (i=0; i < services.length; i++) {
                        name = '<span class="name">' + services[i].title
							+ '</span> (<span class="version">'
							+ services[i].version +
							'</span>)<span class="realname">'
							+ services[i].name + '</span>';

                        str += YAHOO.lang.substitute(EntryRemoveTmpl, {
                            name:  name,
                            desc: services[i].desc,
                            help: "<%= config.servicesRemoveHelp %>"
                        });
                    }
					BPState.log(str);

                    YD.get("servicesList").innerHTML = str;

                };
                
                var ok = BPState.get("activeServices", function(services) {
                    updatePanel(services);
                });
                if (!ok) {
                    updatePanel([]);
                }
            }
        },

        permissions: {
            divId: "permissionsPane",
            buttonId: "permissionsTab", 
            updatePollMS: 3000,
            update:function() {

                var updatePanel = function(perms) {
                    // no permissions comes back as undefined
                    if (typeof perms !== 'object') {
                        perms = []
                    }
                    
                    var i, id, str="", numPerms, numPermSites;

                    numPerms = perms.length;
                    var tmpSiteMap = {}, numPermSites = 0;
                    for (i = 0; i < numPerms; i++) {
                        if (typeof tmpSiteMap[perms[i].site] === 'undefined') {
                            tmpSiteMap[perms[i].site] = true;
                            numPermSites++;
                        }
                    }

                    YD.get("numPerms").innerHTML = numPerms;
                    //YD.get("numPermSites").innerHTML = numPermSites;
                    PermMap = [];
                    for (i = 0; i < numPerms; i++)
                    {
                        id = "permid_" + i;
                        PermMap[id] = {site: perms[i].site, permissionKey: perms[i].permissionKey };

                          str += YAHOO.lang.substitute(EntryRemoveTmpl, {
                                name:  "<span class=\"" + id + "\">" + wbr(perms[i].site) + " (" +  perms[i].whenStr + ")</span>",
                                desc:  (perms[i].allow ? "" : ("<%= config.permissionsDeny %> ")) + perms[i].permission,
                                help: "<%= config.permissionsRemoveHelp %>"
                            });
                    }
                    
                    YD.get("permsList").innerHTML = str;
                };

                var ok = BPState.get("permissions", function(perms) {
                    updatePanel(perms);
                });
                if (!ok) {
                    updatePanel([]);
                }
            }
        },  
        
        troubleshooting: {
            divId: "troubleshootingPane",
            buttonId: "troubleshootingTab",
            updatePollMS: 20000,
            
            update: function() {
                // Can we talk to daemon?
                var poll = this.updatePollMS;
                var timeoutHandle = setTimeout(
                    function() { 
                        YD.get("troubleshootConnectStatus").innerHTML = 
						YAHOO.lang.substitute(
						  "<%= config.nonResponsive %>",
						  {secs: (poll/1000)});
                    }, this.poll);

                BPState.get("activeSites", 
                            function(e) {
                                clearTimeout(timeoutHandle);
                                YD.get("troubleshootConnectStatus").innerHTML = 
								"<%= config.connected %>";
                            });

                // get log sizes (doesn't require daemon connection)
                var s = BPState.getSync("logSize");
                YD.get("troubleshootLogSize").innerHTML = prettySize(s);
            }
        }
    };
  
    var currentView;
    var currentTimeout;

    // register navigation events
    for (var view in views) {
        if (!views.hasOwnProperty(view)) {continue;}

        // setup buttons & pane visibility
        var b = YD.get(views[view].buttonId);
        var v = YD.get(views[view].divId);

        YE.addListener(b, "click", (function() {
            var x = view;
            return function() { 
                ViewMan.show(x); }
        })());
    }

    
    function updateCurrentView() {
        // now update the view
        if (typeof views[currentView].update === "function") {
            views[currentView].update();
        }

        // clear the last poll
        if (currentTimeout) window.clearTimeout(currentTimeout);

        // set up new poll
        if (typeof views[currentView].updatePollMS === "number") {
            currentTimeout = setTimeout(updateCurrentView, views[currentView].updatePollMS);
        }
    }

    return {
        show: function(x) {
            if (!views[x]) throw "No such view: " + x;
            currentView = x;
            
            for (var view in views) {
                if (!views.hasOwnProperty(view)) {continue;}

                // setup buttons & pane visibility
                var b = YD.get(views[view].buttonId);
                var v = YD.get(views[view].divId);

                if (currentView !== view) {
                    // inactive views are grey, clicking calls show
                    YD.removeClass(b, "curnavitem");
                    YD.setStyle(v, "visibility", "hidden");
                } else {
                    YD.addClass(b, "curnavitem");
                    YD.setStyle(v, "visibility", "visible");
                }
            }

            // now update the view
            updateCurrentView();
        }
    };
})();

try {
    // tab is ("general" | "services" | "permissions" |"troubleshooting" )
    var tab = "general";
    ViewMan.show(tab);
} catch (e) {
    var msg = "exception: ";
    for (var k in e) {
        msg += k + ": " + e[k] + ", ";
    }
    alert(msg);
    BPState.log(msg);
}

};  // behavior

if (window.addEventListener) {
    window.addEventListener("load", behavior, true);
} else if (window.attachEvent) {
    window.attachEvent("onload", behavior);
}

