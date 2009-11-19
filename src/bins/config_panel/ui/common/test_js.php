// code to development directly in a browser
// <script>
if (!window.BPState) {
    window.BPState = {
        get: function(key, callback) {
            var val;
            if (key === "activeServices") {
                val = [
                    {
                        type: "built-in",
                        name: "SampleBuiltIn",
                        version: "2.0.7",
                        doc: "A sample built-in service"
                    },
                    {
                        type: "standalone",
                        name: "RubyInterpreter",
                        version: "4.2.3",
                        doc: "A service which allows other services to be written in Ruby"
                    },
                    {
                        type: "standalone",
                        name: "RubyInterpreter",
                        version: "4.2.3",
                        doc: "A service which allows other services to be written in Ruby"
                    },
                    {
                        type: "standalone",
                        name: "RubyInterpreter",
                        version: "4.2.3",
                        doc: "A service which allows other services to be written in Ruby"
                    },
                    {
                        type: "standalone",
                        name: "RubyInterpreter",
                        version: "4.2.3",
                        doc: "A service which allows other services to be written in Ruby"
                    },
                    {
                        type: "standalone",
                        name: "RubyInterpreter",
                        version: "4.2.3",
                        doc: "A service which allows other services to be written in Ruby"
                    },
                    {
                        type: "standalone",
                        name: "RubyInterpreter",
                        version: "4.2.3",
                        doc: "A service which allows other services to be written in Ruby"
                    },
                    {
                        type: "standalone",
                        name: "RubyInterpreter",
                        version: "4.2.3",
                        doc: "A service which allows other services to be written in Ruby"
                    },
                    {
                        type: "standalone",
                        name: "RubyInterpreter",
                        version: "4.2.3",
                        doc: "A service which allows other services to be written in Ruby"
                    },
                    {
                        type: "standalone",
                        name: "RubyInterpreter",
                        version: "4.2.3",
                        doc: "A service which allows other services to be written in Ruby"
                    },
                    {
                        type: "standalone",
                        name: "RubyInterpreter",
                        version: "4.2.3",
                        doc: "A service which allows other services to be written in Ruby"
                    },
                    {
                        type: "standalone",
                        name: "RubyInterpreter",
                        version: "4.2.3",
                        doc: "A service which allows other services to be written in Ruby"
                    },
                    {
                        type: "standalone",
                        name: "RubyInterpreter",
                        version: "4.2.3",
                        doc: "A service which allows other services to be written in Ruby"
                    },
                    {
                        type: "standalone",
                        name: "RubyInterpreter",
                        version: "4.2.3",
                        doc: "A service which allows other services to be written in Ruby"
                    },
                    {
                        type: "standalone",
                        name: "SampleDynamic",
                        version: "14.20.7",
                        doc: "A sample DynamicService that doesn't do anything at all but has a really long description to see what happens visually."
                    }
                ];    
            } else if (key === "activeSites") {
                val = [
                    {
                        site: "BrowserPlus Config Panel",
                        services: [{"service":"DesktopSearch", "version":"1.0.3"},
                                   {"service":"TextToSpeech", "version":"1.1.0"}]
                    },
                    {
                        site: "local file: index.html",
                        services: [{"service":"YahooFinance", "version":"2.0.4"}]
                    }
                 ];
            } else if (key == "permissions") {
                val = [
                    {
                        site: "browserplus.yahoo.com",
                        when: 1234567890,
                        permission: "Use BrowserPlus",
                        permissionKey: "AllowBrowserPlus",
                        allow: false
                    },
                    {
                        site: "facebook.com",
                        when: 1219429081,
                        permission: "Application displays desktop notifications",
                        permissionKey: "DisplayNotifications",
                        allow: true
                    },
                    {
                        site: "facebook.com",
                        when: 1229429081,
                        permission: "Application displays desktop notifications",
                        permissionKey: "DisplayNotifications",
                        allow: true
                    },
                    {
                        site: "facebook.com",
                        when: 1119429081,
                        permission: "Application displays desktop notifications",
                        permissionKey: "DisplayNotifications",
                        allow: true
                    },
                    {
                        site: "facebook.com",
                        when: 1119429081,
                        permission: "Application displays desktop notifications",
                        permissionKey: "DisplayNotifications",
                        allow: true
                    },
                    {
                        site: "facebook.com",
                        when: 1119429081,
                        permission: "Application displays desktop notifications",
                        permissionKey: "DisplayNotifications",
                        allow: true
                    },
                    {
                        site: "facebook.com",
                        when: 1119429081,
                        permission: "Application displays desktop notifications",
                        permissionKey: "DisplayNotifications",
                        allow: true
                    },
                    {
                        site: "facebook.com",
                        when: 1119429081,
                        permission: "Application displays desktop notifications",
                        permissionKey: "DisplayNotifications",
                        allow: true
                    },
                    {
                        site: "facebook.com",
                        when: 1119429081,
                        permission: "Application displays desktop notifications",
                        permissionKey: "DisplayNotifications",
                        allow: true
                    },
                    {
                        site: "facebook.com",
                        when: 1119429081,
                        permission: "Application displays desktop notifications",
                        permissionKey: "DisplayNotifications",
                        allow: true
                    },
                    {
                        site: "facebook.com",
                        when: 1119429081,
                        permission: "Application displays desktop notifications",
                        permissionKey: "DisplayNotifications",
                        allow: true
                    },
                    {
                        site: "facebook.com",
                        when: 1119429081,
                        permission: "Application displays desktop notifications",
                        permissionKey: "DisplayNotifications",
                        allow: true
                    },
                    {
                        site: "facebook.com",
                        when: 1119429081,
                        permission: "Application displays desktop notifications",
                        permissionKey: "DisplayNotifications",
                        allow: true
                    },
                    {
                        site: "facebook.com",
                        when: 1119429081,
                        permission: "Application displays desktop notifications",
                        permissionKey: "DisplayNotifications",
                        allow: true
                    },
                    {
                        site: "facebook.com",
                        when: 1119429081,
                        permission: "Application displays desktop notifications",
                        permissionKey: "DisplayNotifications",
                        allow: true
                    },
                    {
                        site: "facebook.com",
                        when: 1119429081,
                        permission: "Application displays desktop notifications",
                        permissionKey: "DisplayNotifications",
                        allow: true
                    },
                    {
                        site: "flickr.com",
                        when: 1019429081,
                        permission: "Detect photos and other media on attached USB devices",
                        permissionKey: "DetectUSBMedia",
                        allow: true
                    }
                ];
            } else if (key == "requireDomainApproval") {
                 val = true;
            }
                
            callback(val);
            return true;
        },
        
        getSync: function(key) {
            var val = null;
            if (key === "version") {
                val = "2.0.7";
            } else if (key == "enabled") {
                val = (Math.floor(Math.random()*11)) < 5;
            } else if (key == "logSize") {
                val = 2148298;
            }
            return val;
        },
        
        set: function(key, value) {
            var msg = "set " + key + "\n";
            if (key == "permissions") {
                for (var k in value) {
                    msg = msg + value[k].site + ' : ' + value[k].permissionKey + ' : ' + value[k].value + '\n';
                }
            } else if (key == "enabled") {
                msg = msg + value;
            } else if (key == "requireDomainApproval") {
                msg = msg + value;
            }
            alert(msg);
        },
        
        log: function(value) {
            if (typeof console === 'object' &&
                typeof console.log === 'function') {
                console.log("log: + " + value);
            }
        },

        uninstall: function() {alert("Uninstalling B+");},
        restart: function() {alert("Restart B+");},
        deleteService: function(name, version) {alert("Removing service " + name + ", " + version)},
        sendReport: function(includeLogs, message) {alert("Send trouble report");}
    };
}
// </script>

<?php include "main.js" ?>