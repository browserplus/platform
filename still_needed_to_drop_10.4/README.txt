To complete dropping OSX 10.4 support:
 
- merge bakery osx10.5 branch back to maaster
- update sha in external/build.rb to point to new master

- add the following to serc/javascript/src/browserplus.js.erb at about line 640
    } else if (navigator.userAgent.indexOf("Mac OS X 10_4_") != -1) {
        _supportLevel = "unsupported";

- cp exported_symbols.txt to src/bins/browserpluscore

- cp BrowserPlusBuildConfigs.cmake to src/support/build_scripts



