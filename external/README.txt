The BrowserPlus platform uses several 3rd party open source libraries and tools.
This file summarizes the tools used, their purpose, and the licenses under
which they're released.  

3rd party software, when possible, is downloaded directly from the provider
and built on the developers machine as a pre-build step.  

* Boost version 1.41.0 (boost license: http://www.boost.org/users/license.html)
  (C++ abstractions and utilties)
  [http://boost.org] 

* cppunit version 1.12.1 (LGPL)
  (A C++ unit testing framework)
  [http://sourceforge.net/projects/cppunit/]
  NOTE: no binaries linked against this library are shipped to end users

* easylzma version 0.0.7 (public domain)
  (A C library for LZMA (de)compression)
  [http://lloyd.github.org/easylzma]

* ICU version 4.0.1 (ICU license: http://source.icu-project.org/repos/icu/icu/trunk/license.html)
  (A C library to facilitate i18n and L10n)
  [http://site.icu-project.org/]

* jsmin (BSD style license, written by a Yahoo!)
  (A C program to condense or "minify" javascript source)
  http://www.crockford.com/javascript/jsmin.html
  NOTE: no binaries linked against this library are shipped to end users

* libarchive version 2.6.2 (BSD Licensed)
  (A C library which provides flexible implementations of various archiving
   algorithms, such as tar)
  [http://people.freebsd.org/~kientzle/libarchive/]

* libedit version 20090111-3.0 (BSD License)
  (A C library which provides interactive command line editing and history
   facilities - unix only)
  [http://thrysoee.dk/editline/]

* mongoose version 2.8 (MIT license)
  (An embeddable C web server implementation)
  [http://code.google.com/p/mongoose/]

* msft/wtl headers (Microsoft Public License (Ms-PL), formerly the Microsoft "Permissive" License)
  (Several headers which provide wtl support)
  license: http://www.microsoft.com/opensource/licenses.mspx#Ms-PL
  product: http://www.microsoft.com/downloads/details.aspx?familyid=E5BA5BA4-6E6B-462A-B24C-61115E846F0C&displaylang=en

* npapi headers (MPL licensed)
  (header files required to compile NPAPI style plugins that work with
   many modern web browsers -- The canonical maintainer of the headers
   is mozilla, and we redistribute several header files extracted from the
   Mozilla GeckoSDK)
  [https://developer.mozilla.org/En/Gecko_SDK]
 
* openssl version 0.9.8k (BSD-style license)
  (A C library providing implementations of various cryptographic routines)
  [http://openssl.org]

* YAJL version 1.0.4 (BSD license)
    (Clearly, the fastest JSON parser in the world, implemented as a C library)
  [http://lloyd.github.com/yajl/]

* yui compressor version 2.4
  (A command-line JAVA tool for compressing web resources)
  NOTE: this is NOT redistributed to end users
