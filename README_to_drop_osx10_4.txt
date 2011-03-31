To do an osx 10.5 only build:

$ cd external
$ rm -rf Darwin
$ export BP_OSX_TARGET='10.5'
$ ruby build.rb
$ cd ../
$ mkdir build
$ cd build
$ cmake -DOSX10.5_BUILD=true ../src
$ make 

To remove 10.4 support completely:
- edit bakery to remove all "toolchain gcc-4.0" gunk
- cp src/bins/browserpluscore/exported_symbols_10_5.txt to exported_symbols.txt
  and git rm exported_symbols_10_5.txt
- edit the following:
   - src/CMakeLists.txt
       find OSX10.5_BUILD in file, changes should be obvious (and are commented)
   - src/bins/browserpluscore/CMakeLists.txt
       find OSX10.5_BUILD in file, changes should be obvious (and are commented)
   - src/support/build_scripts/BrowserPlusBuildConfigs.cmake
       find OSX10.5_BUILD in file, changes should be obvious (and are commented)

