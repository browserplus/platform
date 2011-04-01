To do an osx 10.4 only build:

$ cd external
$ rm -rf Darwin
$ export BP_OSX_TARGET='10.4'
$ ruby build.rb osx10.4
$ cd ../
$ mkdir build
$ cd build
$ cmake -DOSX10._BUILD=true ../src
$ make 

To remove 10.4 support completely:
- edit bakery to remove all "toolchain gcc-4.0" gunk
- git rm exported_symbols_10_4.txt
- edit the following:
   - src/CMakeLists.txt
       find OSX10.4_BUILD in file, changes should be obvious (and are commented)
   - src/bins/browserpluscore/CMakeLists.txt
       find OSX10.4_BUILD in file, changes should be obvious (and are commented)
   - src/support/build_scripts/BrowserPlusBuildConfigs.cmake
       find OSX10.4_BUILD in file, changes should be obvious (and are commented)
- find any other occurrences of OSX10.4_BUILD that I missed, changes
  should be obvious
