Patches
=======

   * sln_win32.patch is the result of converting 
     cppunit-1.12.1/src/CppUnitLibraries.dsw to a sln.
     It was created by:
        1) unpack the original distro, then rename it cppunit-1.12.1.orig
        2) unpack distro, leaving the name as cppunit-1.12.1
        3) in cmd shell
           cd cppunit-1.12.1\src
           devenv CppUnitLibraries.dsw (answer yes all to conversion question)
           change cppunit project to use /MTd or /MT and /Z7 (debug)
        4) using msys or cygwin
           cd cppunit-1.12.1
           find . -name \*.ncb -o -name \*.suo -o -name \*yourUserName\* |xargs rm
        5) cd Externals\cppunit
           ..\WinTools\bin\diff -PNaur cppunit-1.12.1.orig cppunint-1.12.1 >sln_win32.patch
