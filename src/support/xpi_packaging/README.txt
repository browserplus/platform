Adding in platform installers:

On win32:

use the installer SDK to build a standalone installer with the platform
bpkg built in.  Drop it in this directory as BrowserPlusInstaller.exe.

On osx:

Copy in BrowserPlusInstaller, installer.config, and BrowserPlus.crt
from your favorite installer SDK and distribution server.

Create xpi:
  mac: zip -r <destdir>/browserplus.xpi .
  doze: 7z a -tzip -r <destdir>/browserplus.xpi .

How it works:

At the time the extension is installed, two files are added to the top
level of the extension:

must_run_installer.state
and
must_remove_installer.state

The presence or absence of these files indicates the current
state of execution of the extension:
1. both present - must execute the browserplus installer in the
   background
2. !exist(must_run_installer.state) &&
   exist(must_remove_installer.state)
   we've already spawned the installation in the background, we'll 
   we must delete the extension (note, there's nothing to delay us here
   in case the installer is still running!  On win32 this is not a problem,
   because the binary is already spawned.  on osx.. ?)
3. neither exist -> we're waiting for firefox to be restarted by
   the user, at which point we will no longer exist.  

Notes/Challenges:

In order for a BrowserPlus enabled page to work on first load, for the first
two minutes after browser start we'll refresh plugins at each page load.
(mozilla browsers require refresh() is called after an NPAPI plugin is
 installed).
