// for developer, defines window.BPDialog before including service_install.js
// <script>
// code to allow development directly in a browser
if (!window.BPDialog) {
    window.BPDialog = {

        args: function() {
            return {
                domain: "yahoo.com",

                permissions: [
                    "Use Browser Plus",
                    "Detect photos and other media on attached USB devices",
                    "Application displays desktop notifications"
                ],

                platformUpdates: [
                    {
                        name: "BrowserPlus", 
                        version: "2.0.7", 
                        title: "BrowserPlus Configuration Update",
                        summary: "Provides more user control of BrowserPlus preferences",
                        update: true, 
                        downloadSize: 0 // 0 for updates, positive for installs 
                    }
                ],

                services: [
                    {
                        name: "DesktopSearch", 
                        version: "0.1.7", 
                        title: "Desktop Search",
                        summary: "A corelet to search your desktop using OS facilities.", 
                        update: false, 
                        downloadSize: 49202 // 0 for updates, positive for installs 
                    },
                    {
                        name: "TextToSpeech", 
                        version: "1.0.2", 
                        title: "Text to speech",
                        summary: " corelet that interfaces the OS provided text to speech facilities.", 
                        update: false, 
                        downloadSize: 7311 // 0 for updates, positive for installs 
                    },
                    {
                        name: "Photo Booth", 
                        version: "1.0.3", 
                        title: "Photo Booth",
                        summary: "Capture images from iSight camera (Mac only)", 
                        update: false, 
                        downloadSize: 18320 // 0 for updates, positive for installs 
                    },
                    {
                        name: "JSONRequest", 
                        version: "1.0.5", 
                        title: "JSON Request",
                        summary: "Allows secure cross-domain JSON requests, inspired by http://www.json.org/JSONRequest.html.", 
                        update: true, 
                        downloadSize: 0 // 0 for updates, positive for installs 
                    },
                    {
                        name: "IRCClient", 
                        version: "1.1.7", 
                        title: "IRC Client",
                        summary: "A service that allows you to connect to IRC chat servers.", 
                        update: true, 
                        downloadSize: 0 // 0 for updates, positive for installs 
                    }                ]
            };
        },

        complete: function(result) { alert("complete: " + result);},
        log: function(s) {console.log("BPDialog.log: " + s);},
        show: function(h, w) { }
    };
}
// </script>
<?php include "service_install.js"; ?>