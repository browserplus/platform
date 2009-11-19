Welcome to the sample native BrowserPlus Service!

==== Building on OSX

STEP 1: build it (you'll need gcc and make installed) 
$ make

STEP 2: install it
$ ../../bin/ServiceInstaller -v SampleService

STEP 3: Either set DeveloperMode or restart BrowserPlusCore
(see the top level SDK README.txt for instructions on how to set
 developer mode, or the tutorial for instructions on how to restart
 BrowserPlus (http://browserplus.yahoo.com/developer/service/tutorial/) 

STEP 4: be happy.
You've done it!

==== Building on Win32

STEP 1: Open and build SampleService.sln (you'll need MSVC)

STEP 2: Install it
bpsdk\samples\NativeService> ..\..\bin\ServiceInstaller -v SampleService

STEP 3: Try it out!
Go to the Service Explorer (http://browserplus.yahoo.com/developer/explorer/)
Click "Activated" on left
Click "Sample Service" on left
Click "Test" on right

STEP 4: be happy.
You've done it!
