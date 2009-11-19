Patches
=======
static_crt_and_data_win32.patch
   * Windows-only
   * Use static CRT (/MT, /MTd)
   * Use /Z7 for debug build
   * Generate static libs for code (e.g. icuuc_s.lib)
   * Generate static lib for data (icudt_s.lib)
   * Clients need to #define U_STATIC_IMPLEMENTATION for proper linkage

icudt40l.dat
============
* This file was generated using the tool at:
    http://apps.icu-project.org/datacustom/
* The idea is to include the minimum needed by BP rather than the default 13 MB.

