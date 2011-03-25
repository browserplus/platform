To do an osx 10.5 only build:

$  cd external
$ rm -rf Darwin
$ ruby build.rb osx10.5
$ cd ../
$ mkdir build
$ cd build
$ cmake -DOSX10.5_BUILD=true ../src
$ make 

To remove 10.4 support completely:
- merge bakery osx10.5 branch to master
- cp src/bins/browserpluscore/exported_symbols_10_5.txt to exported_symbols.txt
  and git rm exported_symbols_10_5.txt
- edit the following:
   - external/build.rb
       set bakery_commit to merged sha
   - src/CMakeLists.txt
       find OSX10.5_BUILD in file, changes should be obvious (and are commented)
   - src/bins/browserpluscore/CMakeLists.txt
       find OSX10.5_BUILD in file, changes should be obvious (and are commented)
   - src/support/build_scripts/BrowserPlusBuildConfigs.cmake
       find OSX10.5_BUILD in file, changes should be obvious (and are commented)

# XXX not sure about this, how do we prevent new installs on 10.4?
- add the following to serc/javascript/src/browserplus.js.erb at about line 640
    } else if (navigator.userAgent.indexOf("Mac OS X 10_4_") != -1) {
        _supportLevel = "deprecated";

