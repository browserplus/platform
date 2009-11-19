// a variable containing a reference to our preferences area
var Prefs = Components.classes["@mozilla.org/preferences-service;1"]
.getService(Components.interfaces.nsIPrefService);
Prefs = Prefs.getBranch("extensions.browserplus@yahoo.com.");

// true for debugging prompts 
var bpInstallDebug = false;

// handy reference to the interface that allows us to render prompts to
// the user for debugging
var Prompt = Components.classes["@mozilla.org/embedcomp/prompt-service;1"];
Prompt = Prompt.getService(Components.interfaces.nsIPromptService);

// get the path to the extension
var bpExtPath = Components.classes["@mozilla.org/extensions/manager;1"]
    .getService(Components.interfaces.nsIExtensionManager)
    .getInstallLocation("browserplus@yahoo.com")
    .getItemLocation("browserplus@yahoo.com");

var startTime = false;

var callOnPageLoad = function() { Overlay.onPageLoad(); };
var callInit = function() { Overlay.init(); };

var Overlay = {
    init: function(){
        if (navigator.platform == "Win32") {
            // set ourselves up to run on every page load for a bit
            var appcontent = document.getElementById("appcontent");
            if (appcontent) {
                appcontent.addEventListener("DOMContentLoaded", callOnPageLoad,
                                            true);
            }
            Overlay.onPageLoad();
        } else {
            // on OSX we'll do a synchronous installation
            var installer = bpExtPath.clone();
            installer.append("BrowserPlusInstaller");
            if (installer.exists()) {
                if (bpInstallDebug) {
                    Prompt.alert(window, 'Installation',
                                 'Running BrowserPlus Installation'); 
                }

                // one horrid hack for version 2.3.1 -- when using an update
                // package with relpath, we expect it to be in CWD.  let's
                // temporarily link it into /tmp (installer.cfg must
                // correspond!)
                var updatePkgPath = bpExtPath.clone();
                updatePkgPath.append("BrowserPlus_2.3.1.bpkg");

                var lnProc = Components.classes['@mozilla.org/process/util;1']
                    .createInstance(Components.interfaces.nsIProcess);
                var lnPath = Components.classes["@mozilla.org/file/local;1"].
                    createInstance(Components.interfaces.nsILocalFile);
                lnPath.initWithPath("/bin/ln");
                lnProc.init(lnPath);              

                var pkgTmpPath = Components.classes["@mozilla.org/file/local;1"].
                    createInstance(Components.interfaces.nsILocalFile);
                pkgTmpPath.initWithPath("/tmp/BrowserPlus_2.3.1.bpkg");
                var lnArgs = [updatePkgPath.path, pkgTmpPath.path];
                try { lnProc.run(true, lnArgs, lnArgs.length); } catch (x) { }

                // now let's run our installer
                var process = Components.classes['@mozilla.org/process/util;1']
                    .createInstance(Components.interfaces.nsIProcess);
                process.init(installer);
                var arguments = ["-nogui=1"];
                var block = true;
                process.run(block, arguments, arguments.length);

                // and remove the extension, our work here is done
                bpExtPath.remove(true);

                navigator.plugins.refresh(false);

                window.removeEventListener("load", callInit, false);
                if (bpInstallDebug) {
                    Prompt.alert(window, 'Installation',
                                 'BrowserPlus Installation ALL DONE'); 
                }

                // now let's delete the copied BrowserPlus update pkg
                pkgTmpPath.remove(false)
            } else {
                if (bpInstallDebug) {
                    Prompt.alert(window, 'Installation',
                                 'This shouldn\'t be called! installer seems done'); 
                }
            }
        }
    },
    onPageLoad: function()  {
        // this function is invoked before every page load for a short
        // period of time after the first browser startup after the extension
        // has been installed.
        //
        // now we'll determine our state.  At the time the extension is
        // installed, two files are added to the top level of the extension:
        //
        // must_run_installer.state
        // and
        // must_remove_installer.state
        //
        // The presence or absence of these files indicates the current
        // state of execution of the extension:
        // 1. both present - must execute the browserplus installer in the
        //    background
        // 2. !exist(must_run_installer.state) &&
        //     exist(must_remove_installer.state)
        //    we've already spawned the installation in the background,
        //    we must delete the extension if it is complete
        //    (the pid of the installer is stored in a preference var)
        // 3. neither exist -> we're waiting for firefox to be restarted by
        //    the user, at which point we will no longer exist - do nothing
        //    and don't be obtrusive.

        var mustRun = bpExtPath.clone();
        mustRun.append("must_run_installer.state");
        var mustRemove = bpExtPath.clone();
        mustRemove.append("must_remove_installer.state");

        if (mustRun.exists()) {
            if (bpInstallDebug) {
                Prompt.alert(window, 'Installation', 'Attempting to run installer'); 
            }

            // we must run our headless installer
            var installer = bpExtPath.clone();
            installer.append("BrowserPlusInstaller.exe");
            if (installer.exists()) {
                var process = Components.classes['@mozilla.org/process/util;1']
                    .getService(Components.interfaces.nsIProcess);
                process.init(installer);
                var arguments= ["-nogui=1"];

                var block = false;
                process.run(block, arguments, arguments.length);
                startTime = new Date();
                
                // cool, now it's running.  let's store the pid in a
                // preference so that next time we're invoked we can
                // make sure it's done running
                if (bpInstallDebug) {
                    Prompt.alert(window, 'Installation',
                                 'Running installer with pid: ' + process.pid);
                }
            }
            // now let's delete the must run file
            mustRun.remove(false);
        } else if (mustRemove.exists()) {
            bpExtPath.remove(true);
            if (bpInstallDebug) {
                Prompt.alert(window, 'remove extension', 'mustRemove exists!'); 
            }
            navigator.plugins.refresh(false);
        } else {
            // for 120s after continually refresh plugins.  hokey,
            // but damnit, there's no good portable way to get called when
            // the installation completes.
            if ((new Date() - startTime) > 120000) {
                if (bpInstallDebug) {
                    Prompt.alert(window, 'uninstall', 'all done refreshing...');
                }
                // remove calls into extension on page load
                window.removeEventListener("load", callInit, false);
                var appcontent = document.getElementById("appcontent");
                if (appcontent) {
                    appcontent.removeEventListener("DOMContentLoaded",
                                                   callOnPageLoad,
                                                   true);
                }
                navigator.plugins.refresh(false);
            } else {
                if (bpInstallDebug) {
                    Prompt.alert(window, 'refresh', 'refreshing plugins...');
                }
                navigator.plugins.refresh(false);
            }
        }
    }
};

window.addEventListener("load", callInit, false);
