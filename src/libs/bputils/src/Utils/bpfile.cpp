/**
 * ***** BEGIN LICENSE BLOCK *****
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * The Original Code is BrowserPlus (tm).
 * 
 * The Initial Developer of the Original Code is Yahoo!.
 * Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/*
 *  bpfile.cpp
 *
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 */

#include <set>
#include <map>
#include <sstream>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "api/bpfile.h"

// platform builds get logging
#ifdef BP_PLATFORM_BUILD
#include "api/BPLog.h"
#include "api/bperrorutil.h"
#else
#define BPLOG_DEBUG_STRM(x)
#define BPLOG_INFO_STRM(x)
#define BPLOG_WARN_STRM(x)
#define BPLOG_ERROR_STRM(x)
#endif

#ifdef WIN32
// boost::algorithm::is_any_of causes vs to whine
#pragma warning(disable:4996 4512 4101)
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

#include "boost/filesystem/fstream.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"

#ifdef WIN32
#include <windows.h>
// deal with Windows naming...
#define tStat struct _stat
#define stat(x, y) _wstat(x, y)
#define chmod(x, y) _wchmod(x, y)
#define S_IRUSR _S_IREAD
#define S_IWUSR _S_IWRITE
#else
#define tStat struct stat
#endif

using namespace std;
namespace bfs = boost::filesystem;

namespace bp { namespace file {

const string kFileUrlPrefix("file://");
const string kFolderMimeType("application/x-folder");
const string kLinkMimeType("application/x-link");
const string kBadLinkMimeType("application/x-badlink");

static set<Path> s_delayDelete;

// Extension to mimetype mappings.  Note that
// a single extentsion may have multiple entries.  We will
// build an extension -> mimetypeSet map
//
static const char* s_extensions[] = 
{
    // from http://www.webmaster-toolkit.com/mime-types.shtml
    //
    "3dm", 	"x-world/x-3dmf",
    "3dmf", "x-world/x-3dmf",
    "a", 	"application/octet-stream",
    "aab", 	"application/x-authorware-bin",
    "aam", 	"application/x-authorware-map",
    "aas", 	"application/x-authorware-seg",
    "abc", 	"text/vnd.abc",
    "acgi", "text/html",
    "afl", 	"video/animaflex",
    "ai", 	"application/postscript",
    "aif", 	"audio/aiff",
    "aif", 	"audio/x-aiff",
    "aifc", "audio/aiff",
    "aifc", "audio/x-aiff",
    "aiff", "audio/aiff",
    "aiff", "audio/x-aiff",
    "aim", 	"application/x-aim",
    "aip", 	"text/x-audiosoft-intra",
    "ani", 	"application/x-navi-animation",
    "aos", 	"application/x-nokia-9000-communicator-add-on-software",
    "aps", 	"application/mime",
    "arc", 	"application/octet-stream",
    "arj", 	"application/arj",
    "arj", 	"application/octet-stream",
    "art", 	"image/x-jg",
    "asf", 	"video/x-ms-asf",
    "asm", 	"text/x-asm",
    "asp", 	"text/asp",
    "asx", 	"application/x-mplayer2",
    "asx", 	"video/x-ms-asf",
    "asx", 	"video/x-ms-asf-plugin",
    "au", 	"audio/basic",
    "au", 	"audio/x-au",
    "avi", 	"application/x-troff-msvideo",
    "avi", 	"video/avi",
    "avi", 	"video/msvideo",
    "avi", 	"video/x-msvideo",
    "avs", 	"video/avs-video",
    "bcpio", "application/x-bcpio",
    "bin", 	"application/mac-binary",
    "bin", 	"application/macbinary",
    "bin", 	"application/octet-stream",
    "bin", 	"application/x-binary",
    "bin", 	"application/x-macbinary",
    "bm", 	"image/bmp",
    "bmp", 	"image/bmp",
    "bmp", 	"image/x-windows-bmp",
    "boo", 	"application/book",
    "book", "application/book",
    "boz", 	"application/x-bzip2",
    "bsh", 	"application/x-bsh",
    "bz", 	"application/x-bzip",
    "bz2", 	"application/x-bzip2",
    "c", 	"text/plain",
    "c", 	"text/x-c",
    "c++", 	"text/plain",
    "cat", 	"application/vnd.ms-pki.seccat",
    "cc", 	"text/plain",
    "cc", 	"text/x-c",
    "ccad", "application/clariscad",
    "cco", 	"application/x-cocoa",
    "cdf", 	"application/cdf",
    "cdf", 	"application/x-cdf",
    "cdf", 	"application/x-netcdf",
    "cer", 	"application/pkix-cert",
    "cer", 	"application/x-x509-ca-cert",
    "cha", 	"application/x-chat",
    "chat", "application/x-chat",
    "class","application/java",
    "class","application/java-byte-code",
    "class","application/x-java-class",
    "com", 	"application/octet-stream",
    "com", 	"text/plain",
    "conf", "text/plain",
    "cpio", "application/x-cpio",
    "cpp", 	"text/x-c",
    "cpt", 	"application/mac-compactpro",
    "cpt", 	"application/x-compactpro",
    "cpt", 	"application/x-cpt",
    "crl", 	"application/pkcs-crl",
    "crl", 	"application/pkix-crl",
    "crt", 	"application/pkix-cert",
    "crt", 	"application/x-x509-ca-cert",
    "crt", 	"application/x-x509-user-cert",
    "csh", 	"application/x-csh",
    "csh", 	"text/x-script.csh",
    "css", 	"application/x-pointplus",
    "css", 	"text/css",
    "cxx", 	"text/plain",
    "dcr", 	"application/x-director",
    "deepv","application/x-deepv",
    "def", 	"text/plain",
    "der", 	"application/x-x509-ca-cert",
    "dif", 	"video/x-dv",
    "dir", 	"application/x-director",
    "dl", 	"video/dl",
    "dl", 	"video/x-dl",
    "doc", 	"application/msword",
    "dot", 	"application/msword",
    "dp", 	"application/commonground",
    "drw", 	"application/drafting",
    "dump", "application/octet-stream",
    "dv", 	"video/x-dv",
    "dvi", 	"application/x-dvi",
    "dwf", 	"drawing/x-dwf (old)",
    "dwf", 	"model/vnd.dwf",
    "dwg", 	"application/acad",
    "dwg", 	"image/vnd.dwg",
    "dwg", 	"image/x-dwg",
    "dxf", 	"application/dxf",
    "dxf", 	"image/vnd.dwg",
    "dxf", 	"image/x-dwg",
    "dxr", 	"application/x-director",
    "el", 	"text/x-script.elisp",
    "elc", 	"application/x-bytecode.elisp (compiled elisp)",
    "elc", 	"application/x-elc",
    "env", 	"application/x-envoy",
    "eps", 	"application/postscript",
    "es", 	"application/x-esrehber",
    "etx", 	"text/x-setext",
    "evy", 	"application/envoy",
    "evy", 	"application/x-envoy",
    "exe", 	"application/octet-stream",
    "f", 	"text/plain",
    "f", 	"text/x-fortran",
    "f77", 	"text/x-fortran",
    "f90", 	"text/plain",
    "f90", 	"text/x-fortran",
    "fdf", 	"application/vnd.fdf",
    "fif", 	"application/fractals",
    "fif", 	"image/fif",
    "fli", 	"video/fli",
    "fli", 	"video/x-fli",
    "flo", 	"image/florian",
    "flx", 	"text/vnd.fmi.flexstor",
    "fmf", 	"video/x-atomic3d-feature",
    "for", 	"text/plain",
    "for", 	"text/x-fortran",
    "fpx", 	"image/vnd.fpx",
    "fpx", 	"image/vnd.net-fpx",
    "frl", 	"application/freeloader",
    "funk", "audio/make",
    "g", 	"text/plain",
    "g3", 	"image/g3fax",
    "gif", 	"image/gif",
    "gl", 	"video/gl",
    "gl", 	"video/x-gl",
    "gsd", 	"audio/x-gsm",
    "gsm", 	"audio/x-gsm",
    "gsp", 	"application/x-gsp",
    "gss", 	"application/x-gss",
    "gtar", "application/x-gtar",
    "gz", 	"application/x-compressed",
    "gz", 	"application/x-gzip",
    "gzip", "application/x-gzip",
    "gzip", "multipart/x-gzip",
    "h", 	"text/plain",
    "h", 	"text/x-h",
    "hdf", 	"application/x-hdf",
    "help", "application/x-helpfile",
    "hgl", 	"application/vnd.hp-hpgl",
    "hh", 	"text/plain",
    "hh", 	"text/x-h",
    "hlb", 	"text/x-script",
    "hlp", 	"application/hlp",
    "hlp", 	"application/x-helpfile",
    "hlp", 	"application/x-winhelp",
    "hpg", 	"application/vnd.hp-hpgl",
    "hpgl",	"application/vnd.hp-hpgl",
    "hqx", 	"application/binhex",
    "hqx", 	"application/binhex4",
    "hqx", 	"application/mac-binhex",
    "hqx", 	"application/mac-binhex40",
    "hqx", 	"application/x-binhex40",
    "hqx", 	"application/x-mac-binhex40",
    "hta", 	"application/hta",
    "htc", 	"text/x-component",
    "htm", 	"text/html",
    "html",	"text/html",
    "htmls","text/html",
    "htt", 	"text/webviewhtml",
    "htx", 	"text/html",
    "ice", 	"x-conference/x-cooltalk",
    "ico", 	"image/x-icon",
    "idc", 	"text/plain",
    "ief", 	"image/ief",
    "iefs", "image/ief",
    "iges", "application/iges",
    "iges", "model/iges",
    "igs", 	"application/iges",
    "igs", 	"model/iges",
    "ima", 	"application/x-ima",
    "imap", "application/x-httpd-imap",
    "inf", 	"application/inf",
    "ins", 	"application/x-internett-signup",
    "ip", 	"application/x-ip2",
    "isu", 	"video/x-isvideo",
    "it", 	"audio/it",
    "iv", 	"application/x-inventor",
    "ivr", 	"i-world/i-vrml",
    "ivy", 	"application/x-livescreen",
    "jam", 	"audio/x-jam",
    "jav", 	"text/plain",
    "jav", 	"text/x-java-source",
    "java", "text/plain",
    "java", "text/x-java-source",
    "jcm", 	"application/x-java-commerce",
    "jfif", "image/jpeg",
    "jfif", "image/pjpeg",
    "jfif-tbnl", "image/jpeg",
    "jpe", 	"image/jpeg",
    "jpe", 	"image/pjpeg",
    "jpeg", "image/jpeg",
    "jpeg", "image/pjpeg",
    "jpg", 	"image/jpeg",
    "jpg", 	"image/pjpeg",
    "jps", 	"image/x-jps",
    "js", 	"application/x-javascript",
    "jut", 	"image/jutvision",
    "kar", 	"audio/midi",
    "kar", 	"music/x-karaoke",
    "ksh", 	"application/x-ksh",
    "ksh", 	"text/x-script.ksh",
    "la", 	"audio/nspaudio",
    "la", 	"audio/x-nspaudio",
    "lam", 	"audio/x-liveaudio",
    "latex","application/x-latex",
    "lha", 	"application/lha",
    "lha", 	"application/octet-stream",
    "lha", 	"application/x-lha",
    "lhx", 	"application/octet-stream",
    "list", 	"text/plain",
    "lma", 	"audio/nspaudio",
    "lma", 	"audio/x-nspaudio",
    "log", 	"text/plain",
    "lsp", 	"application/x-lisp",
    "lsp", 	"text/x-script.lisp",
    "lst", 	"text/plain",
    "lsx", 	"text/x-la-asf",
    "ltx", 	"application/x-latex",
    "lzh", 	"application/octet-stream",
    "lzh", 	"application/x-lzh",
    "lzx", 	"application/lzx",
    "lzx", 	"application/octet-stream",
    "lzx", 	"application/x-lzx",
    "m", 	"text/plain",
    "m", 	"text/x-m",
    "m1v", 	"video/mpeg",
    "m2a", 	"audio/mpeg",
    "m2v", 	"video/mpeg",
    "m3u", 	"audio/x-mpequrl",
    "man", 	"application/x-troff-man",
    "map", 	"application/x-navimap",
    "mar", 	"text/plain",
    "mbd", 	"application/mbedlet",
    "mc$", 	"application/x-magic-cap-package-1.0",
    "mcd", 	"application/mcad",
    "mcd", 	"application/x-mathcad",
    "mcf", 	"image/vasa",
    "mcf", 	"text/mcf",
    "mcp", 	"application/netmc",
    "me", 	"application/x-troff-me",
    "mht", 	"message/rfc822",
    "mhtml","message/rfc822",
    "mid", 	"application/x-midi",
    "mid", 	"audio/midi",
    "mid", 	"audio/x-mid",
    "mid", 	"audio/x-midi",
    "mid", 	"music/crescendo",
    "mid", 	"x-music/x-midi",
    "midi", "application/x-midi",
    "midi", "audio/midi",
    "midi", "audio/x-mid",
    "midi", "audio/x-midi",
    "midi", "music/crescendo",
    "midi", "x-music/x-midi",
    "mif", 	"application/x-frame",
    "mif", 	"application/x-mif",
    "mime", "message/rfc822",
    "mime", "www/mime",
    "mjf", 	"audio/x-vnd.audioexplosion.mjuicemediafile",
    "mjpg", "video/x-motion-jpeg",
    "mm", 	"application/base64",
    "mm", 	"application/x-meme",
    "mme", 	"application/base64",
    "mod", 	"audio/mod",
    "mod", 	"audio/x-mod",
    "moov", "video/quicktime",
    "mov", 	"video/quicktime",
    "movie","video/x-sgi-movie",
    "mp2", 	"audio/mpeg",
    "mp2", 	"audio/x-mpeg",
    "mp2", 	"video/mpeg",
    "mp2", 	"video/x-mpeg",
    "mp2", 	"video/x-mpeq2a",
    "mp3", 	"audio/mpeg3",
    "mp3", 	"audio/x-mpeg-3",
    "mp3", 	"video/mpeg",
    "mp3", 	"video/x-mpeg",
    "mpa", 	"audio/mpeg",
    "mpa", 	"video/mpeg",
    "mpc", 	"application/x-project",
    "mpe", 	"video/mpeg",
    "mpeg", "video/mpeg",
    "mpg", 	"audio/mpeg",
    "mpg", 	"video/mpeg",
    "mpga", "audio/mpeg",
    "mpp", 	"application/vnd.ms-project",
    "mpt", 	"application/x-project",
    "mpv", 	"application/x-project",
    "mpx", 	"application/x-project",
    "mrc", 	"application/marc",
    "ms", 	"application/x-troff-ms",
    "mv", 	"video/x-sgi-movie",
    "my", 	"audio/make",
    "mzz", 	"application/x-vnd.audioexplosion.mzz",
    "nap", 	"image/naplps",
    "naplps", "image/naplps",
    "nc", 	"application/x-netcdf",
    "ncm", 	"application/vnd.nokia.configuration-message",
    "nif", 	"image/x-niff",
    "niff", "image/x-niff",
    "nix", 	"application/x-mix-transfer",
    "nsc", 	"application/x-conference",
    "nvd", 	"application/x-navidoc",
    "o", 	"application/octet-stream",
    "oda", 	"application/oda",
    "omc", 	"application/x-omc",
    "omcd", "application/x-omcdatamaker",
    "omcr", "application/x-omcregerator",
    "p", 	"text/x-pascal",
    "p10", 	"application/pkcs10",
    "p10", 	"application/x-pkcs10",
    "p12", 	"application/pkcs-12",
    "p12", 	"application/x-pkcs12",
    "p7a", 	"application/x-pkcs7-signature",
    "p7c", 	"application/pkcs7-mime",
    "p7c", 	"application/x-pkcs7-mime",
    "p7m", 	"application/pkcs7-mime",
    "p7m", 	"application/x-pkcs7-mime",
    "p7r", 	"application/x-pkcs7-certreqresp",
    "p7s", 	"application/pkcs7-signature",
    "part", 	"application/pro_eng",
    "pas", 	"text/pascal",
    "pbm", 	"image/x-portable-bitmap",
    "pcl", 	"application/vnd.hp-pcl",
    "pcl", 	"application/x-pcl",
    "pct", 	"image/x-pict",
    "pcx", 	"image/x-pcx",
    "pdb", 	"chemical/x-pdb",
    "pdf", 	"application/pdf",
    "pfunk", "audio/make",
    "pfunk", "audio/make.my.funk",
    "pgm", 	"image/x-portable-graymap",
    "pgm", 	"image/x-portable-greymap",
    "pic", 	"image/pict",
    "pict", "image/pict",
    "pkg", 	"application/x-newton-compatible-pkg",
    "pko", 	"application/vnd.ms-pki.pko",
    "pl", 	"text/plain",
    "pl", 	"text/x-script.perl",
    "plx", 	"application/x-pixclscript",
    "pm", 	"image/x-xpixmap",
    "pm", 	"text/x-script.perl-module",
    "pm4", 	"application/x-pagemaker",
    "pm5", 	"application/x-pagemaker",
    "png", 	"image/png",
    "pnm", 	"application/x-portable-anymap",
    "pnm", 	"image/x-portable-anymap",
    "pot", 	"application/mspowerpoint",
    "pot", 	"application/vnd.ms-powerpoint",
    "pov", 	"model/x-pov",
    "ppa", 	"application/vnd.ms-powerpoint",
    "ppm", 	"image/x-portable-pixmap",
    "pps", 	"application/mspowerpoint",
    "pps", 	"application/vnd.ms-powerpoint",
    "ppt", 	"application/mspowerpoint",
    "ppt", 	"application/powerpoint",
    "ppt", 	"application/vnd.ms-powerpoint",
    "ppt", 	"application/x-mspowerpoint",
    "ppz", 	"application/mspowerpoint",
    "pre", 	"application/x-freelance",
    "prt", 	"application/pro_eng",
    "ps", 	"application/postscript",
    "psd", 	"application/octet-stream",
    "pvu", 	"paleovu/x-pv",
    "pwz", 	"application/vnd.ms-powerpoint",
    "py", 	"text/x-script.phyton",
    "pyc", 	"applicaiton/x-bytecode.python",
    "qcp", 	"audio/vnd.qcelp",
    "qd3", 	"x-world/x-3dmf",
    "qd3d", "x-world/x-3dmf",
    "qif", 	"image/x-quicktime",
    "qt", 	"video/quicktime",
    "qtc", 	"video/x-qtc",
    "qti", 	"image/x-quicktime",
    "qtif", "image/x-quicktime",
    "ra", 	"audio/x-pn-realaudio",
    "ra", 	"audio/x-pn-realaudio-plugin",
    "ra", 	"audio/x-realaudio",
    "ram", 	"audio/x-pn-realaudio",
    "ras", 	"application/x-cmu-raster",
    "ras", 	"image/cmu-raster",
    "ras", 	"image/x-cmu-raster",
    "rast", "image/cmu-raster",
    "rexx", "text/x-script.rexx",
    "rf", 	"image/vnd.rn-realflash",
    "rgb", 	"image/x-rgb",
    "rm", 	"application/vnd.rn-realmedia",
    "rm", 	"audio/x-pn-realaudio",
    "rmi", 	"audio/mid",
    "rmm", 	"audio/x-pn-realaudio",
    "rmp", 	"audio/x-pn-realaudio",
    "rmp", 	"audio/x-pn-realaudio-plugin",
    "rng", 	"application/ringing-tones",
    "rng", 	"application/vnd.nokia.ringing-tone",
    "rnx", 	"application/vnd.rn-realplayer",
    "roff", 	"application/x-troff",
    "rp", 	"image/vnd.rn-realpix",
    "rpm", 	"audio/x-pn-realaudio-plugin",
    "rt", 	"text/richtext",
    "rt", 	"text/vnd.rn-realtext",
    "rtf", 	"application/rtf",
    "rtf", 	"application/x-rtf",
    "rtf", 	"text/richtext",
    "rtx", 	"application/rtf",
    "rtx", 	"text/richtext",
    "rv", 	"video/vnd.rn-realvideo",
    "s", 	"text/x-asm",
    "s3m", 	"audio/s3m",
    "saveme", "application/octet-stream",
    "sbk", 	"application/x-tbook",
    "scm", 	"application/x-lotusscreencam",
    "scm", 	"text/x-script.guile",
    "scm", 	"text/x-script.scheme",
    "scm", 	"video/x-scm",
    "sdml", "text/plain",
    "sdp", 	"application/sdp",
    "sdp", 	"application/x-sdp",
    "sdr", 	"application/sounder",
    "sea", 	"application/sea",
    "sea", 	"application/x-sea",
    "set", 	"application/set",
    "sgm", 	"text/sgml",
    "sgm", 	"text/x-sgml",
    "sgml", "text/sgml",
    "sgml", "text/x-sgml",
    "sh", 	"application/x-bsh",
    "sh", 	"application/x-sh",
    "sh", 	"application/x-shar",
    "sh", 	"text/x-script.sh",
    "shar", "application/x-bsh",
    "shar", "application/x-shar",
    "shtml", "text/html",
    "shtml", "text/x-server-parsed-html",
    "sid", 	"audio/x-psid",
    "sit", 	"application/x-sit",
    "sit", 	"application/x-stuffit",
    "skd", 	"application/x-koan",
    "skm", 	"application/x-koan",
    "skp", 	"application/x-koan",
    "skt", 	"application/x-koan",
    "sl", 	"application/x-seelogo",
    "smi", 	"application/smil",
    "smil", "application/smil",
    "snd", 	"audio/basic",
    "snd", 	"audio/x-adpcm",
    "sol", 	"application/solids",
    "spc", 	"application/x-pkcs7-certificates",
    "spc", 	"text/x-speech",
    "spl", 	"application/futuresplash",
    "spr", 	"application/x-sprite",
    "sprite", "application/x-sprite",
    "src", 	"application/x-wais-source",
    "ssi", 	"text/x-server-parsed-html",
    "ssm", 	"application/streamingmedia",
    "sst", 	"application/vnd.ms-pki.certstore",
    "step", "application/step",
    "stl", 	"application/sla",
    "stl", 	"application/vnd.ms-pki.stl",
    "stl", 	"application/x-navistyle",
    "stp", 	"application/step",
    "sv4cpio", "application/x-sv4cpio",
    "sv4crc",  "application/x-sv4crc",
    "svf", 	"image/vnd.dwg",
    "svf", 	"image/x-dwg",
    "svr", 	"application/x-world",
    "svr", 	"x-world/x-svr",
    "swf", 	"application/x-shockwave-flash",
    "t", 	"application/x-troff",
    "talk", "text/x-speech",
    "tar", 	"application/x-tar",
    "tbk", 	"application/toolbook",
    "tbk", 	"application/x-tbook",
    "tcl", 	"application/x-tcl",
    "tcl", 	"text/x-script.tcl",
    "tcsh", "text/x-script.tcsh",
    "tex", 	"application/x-tex",
    "texi", "application/x-texinfo",
    "texinfo", "application/x-texinfo",
    "text", "application/plain",
    "text", "text/plain",
    "tgz", 	"application/gnutar",
    "tgz", 	"application/x-compressed",
    "tif", 	"image/tiff",
    "tif", 	"image/x-tiff",
    "tiff", "image/tiff",
    "tiff", "image/x-tiff",
    "tr", 	"application/x-troff",
    "tsi", 	"audio/tsp-audio",
    "tsp", 	"application/dsptype",
    "tsp", 	"audio/tsplayer",
    "tsv", 	"text/tab-separated-values",
    "turbot", "image/florian",
    "txt", 	"text/plain",
    "uil", 	"text/x-uil",
    "uni", 	"text/uri-list",
    "unis", "text/uri-list",
    "unv", 	"application/i-deas",
    "uri", 	"text/uri-list",
    "uris", "text/uri-list",
    "ustar", "application/x-ustar",
    "ustar", "multipart/x-ustar",
    "uu", 	"application/octet-stream",
    "uu", 	"text/x-uuencode",
    "uue", 	"text/x-uuencode",
    "vcd", 	"application/x-cdlink",
    "vcs", 	"text/x-vcalendar",
    "vda", 	"application/vda",
    "vdo", 	"video/vdo",
    "vew", 	"application/groupwise",
    "viv", 	"video/vivo",
    "viv", 	"video/vnd.vivo",
    "vivo", "video/vivo",
    "vivo", "video/vnd.vivo",
    "vmd", 	"application/vocaltec-media-desc",
    "vmf", 	"application/vocaltec-media-file",
    "voc", 	"audio/voc",
    "voc", 	"audio/x-voc",
    "vos", 	"video/vosaic",
    "vox", 	"audio/voxware",
    "vqe", 	"audio/x-twinvq-plugin",
    "vqf", 	"audio/x-twinvq",
    "vql", 	"audio/x-twinvq-plugin",
    "vrml", "application/x-vrml",
    "vrml", "model/vrml",
    "vrml", "x-world/x-vrml",
    "vrt", 	"x-world/x-vrt",
    "vsd", 	"application/x-visio",
    "vst", 	"application/x-visio",
    "vsw", 	"application/x-visio",
    "w60", 	"application/wordperfect6.0",
    "w61", 	"application/wordperfect6.1",
    "w6w", 	"application/msword",
    "wav", 	"audio/wav",
    "wav", 	"audio/x-wav",
    "wb1", 	"application/x-qpro",
    "wbmp", "image/vnd.wap.wbmp",
    "web", 	"application/vnd.xara",
    "wiz", 	"application/msword",
    "wk1", 	"application/x-123",
    "wmf", 	"windows/metafile",
    "wml", 	"text/vnd.wap.wml",
    "wmlc", "application/vnd.wap.wmlc",
    "wmls", "text/vnd.wap.wmlscript",
    "wmlsc", "application/vnd.wap.wmlscriptc",
    "word", "application/msword",
    "wp", 	"application/wordperfect",
    "wp5", 	"application/wordperfect",
    "wp5", 	"application/wordperfect6.0",
    "wp6", 	"application/wordperfect",
    "wpd", 	"application/wordperfect",
    "wpd", 	"application/x-wpwin",
    "wq1", 	"application/x-lotus",
    "wri", 	"application/mswrite",
    "wri", 	"application/x-wri",
    "wrl", 	"application/x-world",
    "wrl", 	"model/vrml",
    "wrl", 	"x-world/x-vrml",
    "wrz", 	"model/vrml",
    "wrz", 	"x-world/x-vrml",
    "wsc", 	"text/scriplet",
    "wsrc", "application/x-wais-source",
    "wtk", 	"application/x-wintalk",
    "xbm", 	"image/x-xbitmap",
    "xbm", 	"image/x-xbm",
    "xbm", 	"image/xbm",
    "xdr", 	"video/x-amt-demorun",
    "xgz", 	"xgl/drawing",
    "xif", 	"image/vnd.xiff",
    "xl", 	"application/excel",
    "xla", 	"application/excel",
    "xla", 	"application/x-excel",
    "xla", 	"application/x-msexcel",
    "xlb", 	"application/excel",
    "xlb", 	"application/vnd.ms-excel",
    "xlb", 	"application/x-excel",
    "xlc", 	"application/excel",
    "xlc", 	"application/vnd.ms-excel",
    "xlc", 	"application/x-excel",
    "xld", 	"application/excel",
    "xld", 	"application/x-excel",
    "xlk", 	"application/excel",
    "xlk", 	"application/x-excel",
    "xll", 	"application/excel",
    "xll", 	"application/vnd.ms-excel",
    "xll", 	"application/x-excel",
    "xlm", 	"application/excel",
    "xlm", 	"application/vnd.ms-excel",
    "xlm", 	"application/x-excel",
    "xls", 	"application/excel",
    "xls", 	"application/vnd.ms-excel",
    "xls", 	"application/x-excel",
    "xls", 	"application/x-msexcel",
    "xlt", 	"application/excel",
    "xlt", 	"application/x-excel",
    "xlv", 	"application/excel",
    "xlv", 	"application/x-excel",
    "xlw", 	"application/excel",
    "xlw", 	"application/vnd.ms-excel",
    "xlw", 	"application/x-excel",
    "xlw", 	"application/x-msexcel",
    "xm", 	"audio/xm",
    "xml", 	"application/xml",
    "xml", 	"text/xml",
    "xmz", 	"xgl/movie",
    "xpix", "application/x-vnd.ls-xpix",
    "xpm", 	"image/x-xpixmap",
    "xpm", 	"image/xpm",
    "x-png","image/png",
    "xsr", 	"video/x-amt-showrun",
    "xwd", 	"image/x-xwd",
    "xwd", 	"image/x-xwindowdump",
    "xyz", 	"chemical/x-pdb",
    "z", 	"application/x-compress",
    "z", 	"application/x-compressed",
    "zip", 	"application/x-compressed",
    "zip", 	"application/x-zip-compressed",
    "zip", 	"application/zip",
    "zip", 	"multipart/x-zip",
    "zoo", 	"application/octet-stream",
    "zsh", 	"text/x-script.zsh",

    // from http://support.microsoft.com/kb/288102
    //
    "asf",	"video/x-ms-asf",
    "asx",	"video/x-ms-asf",
    "wma",	"audio/x-ms-wma",
    "wax",	"audio/x-ms-wax",
    "wmv",	"audio/x-ms-wmv",
    "wvx",	"video/x-ms-wvx",
    "wm",	"video/x-ms-wm",
    "wmx",	"video/x-ms-wmx",
    "wmz",	"application/x-ms-wmz",
    "wmd",	"application/x-ms-wmd",

    // from http://developer.apple.com/documentation/AppleApplications/Reference/SafariWebContent/CreatingContentforSafarioniPhone/chapter_2_section_11.html
    //
    "3gp",     "audio/3gpp",
    "3gpp",    "audio/3gpp",
    "3g2",     "audio/3gpp2",
    "3gp2",    "audio/3gpp2",
    "aiff",	   "audio/aiff",
    "aif",     "audio/aiff",
    "aifc",    "audio/aiff",
    "cdda",    "audio/aiff",
    "aiff",	   "audio/x-aiff",
    "aif",     "audio/x-aiff",
    "aifc",    "audio/x-aiff",
    "cdda",    "audio/x-aiff",
    "amr",	   "audio/amr",
    "mp3",	   "audio/mp3",
    "mp3",     "audio/mpeg3",
    "mp3",     "audio/x-mp3",
    "mp3",     "audio/x-mpeg3",
    "swa",	   "audio/mp3",
    "swa",     "audio/mpeg3",
    "swa",     "audio/x-mp3",
    "swa",     "audio/x-mpeg3",
    "mp4",     "audio/mp4",
    "mpeg",    "audio/mpeg",
    "mpg",     "audio/mpeg",
    "mp3",     "audio/mpeg",
    "swa",     "audio/mpeg",
    "mpeg",    "audio/x-mpeg",
    "mpg",     "audio/x-mpeg",
    "mp3",     "audio/x-mpeg",
    "swa",     "audio/x-mpeg",
    "wav",     "audio/wav",
    "wav",     "audio/x-wav",
    "bwf",     "audio/wav",
    "bwf",     "audio/x-wav",
    "m4a",     "audio/x-m4a",
    "m4b",     "audio/x-m4b",
    "m4p",     "audio/x-m4p",
    "3gp",     "video/3gpp",
    "3gpp",    "video/3gpp",
    "3g2",     "video/3gpp2",
    "3gp2",    "video/3gpp2",
    "mp4",     "video/mp4",
    "mov",     "video/quicktime",
    "qt",      "video/quicktime",
    "mqv",     "video/quicktime",
    "m4v",     "video/x-m4v",

     // From http://filext.com
    "icf",     "text/calendar",
    "vcf",     "text/calendar",
    "tlz",     "application/x-lzip",
    "lz",      "application/x-lzip",
    // should also be .tar.lz, but we use rfind to get extension
    
    // Null termination is important
    NULL,      NULL
};

    
// case-insensitive key comparisons for mimetypes
struct MyCompare {
    bool operator()(const string& lhs, const string& rhs) const {
        return(strcasecmp(lhs.c_str(), rhs.c_str()) < 0);
    }
};
    
typedef map<string, set<string>, MyCompare> TExtMap;
static TExtMap s_extensionMap;
    
static void
initializeMimeTypes()
{
    static bool initialized = false;
    if (initialized) return;
    
    size_t i = 0;
    while (s_extensions[i] != NULL) {
        string ext = s_extensions[i++];
        string mimetype = s_extensions[i++];
        set<string>& s = s_extensionMap[ext];
        s.insert(mimetype);
    }
    initialized = true;
}


class DirEntry {
public:
    static DirEntry fromPath(const Path& p) {
        DirEntry rval;
        FileInfo fi;
        if (statFile(p, fi)) {
            rval.m_path = p;
            rval.m_deviceId = fi.deviceId;
            rval.m_fileIdHigh = fi.fileIdHigh;
            rval.m_fileIdLow = fi.fileIdLow;
        }
        return rval;
    }
    DirEntry() : m_path(), m_deviceId(0), 
        m_fileIdHigh(0), m_fileIdLow(0) {}
    Path m_path;
    boost::uint32_t m_deviceId;
    boost::uint32_t m_fileIdHigh;
    boost::uint32_t m_fileIdLow;
};

// Test for circular links.  We look for p's deviceId/fileId on the
// stack of dirs we're currently visiting.  If we find it,
// we don't revisit.
//
static bool
isCircular(const bp::file::Path& p,
           const vector<DirEntry>& stack)
{
    FileInfo fi;
    if (!statFile(p, fi)) {
        return false;
    }

    for (size_t i = 0; i < stack.size(); ++i) {
        if (stack[i].m_path.empty()) {
            // somehow got an empty entry, ignore it
            continue;
        }
        if (stack[i].m_deviceId == fi.deviceId 
            && stack[i].m_fileIdHigh == fi.fileIdHigh
            && stack[i].m_fileIdLow == fi.fileIdLow) {
            stringstream ss;
            ss << "circular link to " << p << " found, directory stack is:" << endl;
            for (size_t j = 0; j < stack.size(); ++j) {
                ss << stack[j].m_path << "(" << stack[j].m_deviceId
                   << ", " << stack[j].m_fileIdHigh
                   << ", " << stack[j].m_fileIdLow << ")" << endl;
            }
            BPLOG_WARN_STRM(ss.str());
            return true;
        }
    }
    return false;
}


// Common code for visit() and recursiveVisit(), knows all about
// links, circular link detection, and constructing a 
// relative pseudo-path for each visited node.
//
static bool
doVisit(const bp::file::Path& p,          // node to visit
        bp::file::IVisitor& v,            // visitor to apply
        vector<DirEntry>& pathStack,      // stack of dirs currently being visited
        bp::file::Path& relativeDir,      // current relative pseudo directory name
        bool followLinks,                 // follow links?
        bool recursive)                   // recurse into children?
{
    // Find real target if we're chasing links.
    bool islink = isLink(p);
    Path target = p;
    if (followLinks && islink) {
        // broken links will visit link itself
        if (!resolveLink(p, target)) {
            target = p;
        }
    }

    // don't revisit circular links
    if (isCircular(p, pathStack)) {
        return true;
    }

    // visit this node
    if (v.visitNode(target, relativeDir/p.filename()) == IVisitor::eStop) {
        return false;
    }

    // all done if this is a link and we're not chasing
    if (islink && !followLinks) {
        return true;
    }

    // visit a directory's children
    if (bfs::is_directory(target)) {
        if (recursive) {
            // remember ourselves for cycle detection
            pathStack.push_back(DirEntry::fromPath(target));

            // visit all children
            int lastLevel = -1;
            tRecursiveDirIter end;
            for (tRecursiveDirIter iter(target); iter != end; ++iter) {
                // resolve link if chasing
                Path node(iter->path());
                Path nodeTarget = node;
                if (followLinks && isLink(node)) {
                    if (!resolveLink(node, nodeTarget)) {
                        // broken links will visit link itself
                        nodeTarget = node;
                    }
                }

                // check for cycles
                if (bfs::is_directory(nodeTarget)) {
                    if (isCircular(nodeTarget, pathStack)) {
                        iter.no_push();
                        continue;
                    }
                }

                // Maintain pathStack and relativeDir.  Iterator 
                // descent happens one level at a time, ascent can be 
                // multiple levels
                int thisLevel = iter.level();
                if (thisLevel > lastLevel) {
                    if (thisLevel - lastLevel > 1) {
                        // this was unexpected!
                        stringstream ss;
                        ss << "Unexpected level change, " << lastLevel 
                           << " -> " << thisLevel << ", node = " << node
                           << ", relativeDir = " << relativeDir;
                        throw ss.str();
                    }
                    pathStack.push_back(DirEntry::fromPath(nodeTarget));
                    relativeDir /= node.parent_path().filename();
                } else if (thisLevel < lastLevel) {
                    for (int i = 0; i < lastLevel - thisLevel; i++) {
                        pathStack.pop_back();
                        relativeDir = relativeDir.parent_path();
                    }
                }
                lastLevel = thisLevel;

                // visit child node
                if (v.visitNode(nodeTarget, relativeDir/node.filename()) == IVisitor::eStop) {
                    return false;
                }

                // Links to dirs take special consideration.  If we're not chasing
                // links, we don't dive into the directory.  If we're chasing
                // links and this is a non-symlink link (shortcut or alias), 
                // we must manually recurse into it since the bfs iterator won't.
                // Symlinks to dirs are handled by bfs iterator.
                if (isLink(node) && bfs::is_directory(nodeTarget)) {
                    if (!followLinks) {
                        if (isSymlink(node)) {
                            iter.no_push();
                        }
                    } else {
                        if (!isSymlink(node)) {
                            class DelegatingVisitor : virtual public IVisitor {
                            public:
                                DelegatingVisitor(IVisitor& v) : m_delegate(v) {}
                                virtual ~DelegatingVisitor() {}
                                tResult visitNode(const Path& p, 
                                                  const Path& r) {
                                    return m_delegate.visitNode(p, r);
                                }
                                IVisitor& m_delegate;
                            };
                            pathStack.push_back(DirEntry::fromPath(nodeTarget));
                            relativeDir /= node.filename();
                            DelegatingVisitor dv(v);
                            tRecursiveDirIter end2;
                            for (tRecursiveDirIter iter2(nodeTarget); iter2 != end2; ++iter2) {
                                Path kid(iter2->path());
                                if (!doVisit(kid, dv, pathStack, relativeDir,
                                             followLinks, true)) {
                                    return false;
                                }
                            }
                            pathStack.pop_back();
                            relativeDir = relativeDir.parent_path();
                        }
                    }
                }
            }
            pathStack.pop_back();
            relativeDir = relativeDir.parent_path();
        } else {
            relativeDir /= p.parent_path().filename();
            tDirIter end;
            for (tDirIter iter(target); iter != end; ++iter) {
                Path node(iter->path());
                Path nodeTarget = node;
                // resolve link if chasing
                if (followLinks && isLink(node)) {
                    if (!resolveLink(node, nodeTarget)) {
                        // broken links will visit link itself
                        nodeTarget = node;
                    }
                }
                // visit the child
                if (v.visitNode(nodeTarget, relativeDir/node.filename()) == IVisitor::eStop) {
                    return false;
                }
            }
            relativeDir = relativeDir.parent_path();
        }
    }
    return true;
}


static string 
urlEncode(const string& s)
{
    string out;
    char hex[4];

    static const char noencode[] = "!'()*-._";
    static const char hexvals[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
        'A', 'B', 'C', 'D', 'E', 'F'
    };
    
    for (unsigned int i = 0; i < s.length(); i++) {
        if (isalnum((unsigned char)s[i]) || strchr(noencode, s[i]) != NULL) {
            out.append(&s[i], 1);
        } else {
            hex[0] = '%';
            hex[1] = hexvals[(s[i] >> 4) & 0x0F];
            hex[2] = hexvals[s[i] & 0xF];
            hex[3] = 0;
            out.append(hex, strlen(hex));
        }
    }
    return out;
}


static char 
char2hex(char c)
{
  if (isdigit(c)) c -= '0';
  else c = (c | 0x20) - 'a' + 10;
  return c;
}


/* collapse %XX into the ascii char it represents IN PLACE */
static string
urlDecode(string s)
{
    unsigned int i = 0;
    unsigned char hv;

    for (i=0; i < s.size(); i++) {
        if (s.at(i) == '%') {
            if (isxdigit(s.at(i+1)) && isxdigit(s.at(i+2))) {
                hv = char2hex(s.at(i+1)) << 4;
                 hv = hv + (unsigned char) char2hex(s.at(i+2));
                s.replace(i, 3, (const char *) &hv, 1);
            }
        } else if (s.at(i) == '+') {
            hv = ' ';
            s.replace(i, 1, (const char *) &hv, 1);
        }
    }
    return s;
}


//-------------------------- Path 

#ifdef WIN32
Path::Path(const std::string& utf8) : tBase(bp::file::nativeFromUtf8(utf8)) 
{
}
#endif


string 
Path::utf8() const
{
    return utf8FromNative(string());
}


string 
Path::externalUtf8() const
{
    return utf8FromNative(external_file_string());
}


string 
Path::url() const
{
    // Split out edges.  Note that if "s" started
    // with "/", first edge will be empty.
    std::string s = utf8();
    if (s.empty()) {
        return std::string("");
    };
    vector<std::string> edges;
    boost::algorithm::split(edges, s, boost::algorithm::is_any_of("/"));
    std::string rval = kFileUrlPrefix;
    unsigned int start = 0;
    if (edges[0].empty()) {
        start++;
    }
#ifdef WIN32
    // Windows hosts appear differently in pathname (//host/path)
    // and must appear in final url as file://host/path.  Also
    // want drive names, which appear as C:/foo, to appear in
    // url as file:///C:/foo
    if (edges[start].length() == 0) {
        // have a host, e.g. //host/path
        rval.append(edges[++start]);
        start++;
    } else if (edges[start].length() == 2 && edges[start][1] == ':') {
        // have a drive, e.g. c:/foo
        rval.append("/" + edges[start]);
        start++;
    }
#endif

    // add remaining edges
    for (unsigned int i = start; i < edges.size(); i++) {
        if (edges[i].length() > 0) {
            rval.append("/");
            rval.append(urlEncode(edges[i]));
        }
    }
    return rval;
}


Path 
Path::canonical() const
{
    tString dot = nativeFromUtf8(".");
    tString dotdot = nativeFromUtf8("..");
    Path rval;
    for (iterator iter(begin()); iter != end(); ++iter) {
        if (iter->compare(dot) == 0) {
            continue;
        }
        if (iter->compare(dotdot) == 0) {
            rval = rval.parent_path();
            continue;
        }
        rval = rval / *iter;
    }

    // make sure we preserve a trailing /
    tString slash = nativeFromUtf8("/");
    if (string().rfind(slash) == string().length()-1) {
        tString s = rval.string();
        s += slash;
        rval = s;
    }
    return rval;
}



Path
Path::relativeTo(const Path& base) const
{
    if (base == *this) {
        return Path();
    }

    std::string baseStr = base.utf8();
    std::string ourStr = utf8();
    if (baseStr.rfind("/") != baseStr.length()-1) {
        baseStr += "/";
    }
    if (ourStr.find(baseStr) != 0) {
        boost::system::error_code ec;
        throw tFileSystemError("path1 not relative to path2",
                               *this, base, ec);
    }
    tString relStr = nativeFromUtf8(ourStr.substr(baseStr.length(),
                                                  string::npos));
    return Path(relStr);
}

//-------------------------- convenience functions


bool
visit(const Path& p,
      IVisitor& v,
      bool followLinks)
{
    vector<DirEntry> stack;
    Path rp;
    return doVisit(p, v, stack, rp, followLinks, false);
}


bool
recursiveVisit(const Path& p,
               IVisitor& v,
               bool followLinks)
{
    vector<DirEntry> stack;
    Path rp;
    return doVisit(p, v, stack, rp, followLinks, true);
}


Path 
pathFromURL(const string& url)
{
    Path rval;
    
    // file url format is file://host/path

    // check for file://
    if (url.substr(0, kFileUrlPrefix.length()) != kFileUrlPrefix) {
        return rval;
    }

    // Rip off file:// and get remaining edges.
    // Note that if "s" started with "/", 
    // first edge will be empty.  Also, Windows 
    // handles hosts with //host/path.  No uniform
    // way to do hosts on other platforms, so bail.
    string s = url.substr(kFileUrlPrefix.length());
    vector<string> edges;
    boost::algorithm::split(edges, s, boost::algorithm::is_any_of("/"));
    unsigned int start = 0;
    if (edges[0].empty()) {
        start++;
    }

    string firstEdge = urlDecode(edges[start]);
    if (s[0] == '/') {
#ifdef WIN32
        // no host, check for drive
        if (firstEdge.length() == 2 && firstEdge[1] == ':') {
            rval = Path(firstEdge + "/");
        } else {
            rval = Path("/" + firstEdge);
        }
#else
        rval = Path("/" + firstEdge);
#endif
        start++;
    } else {
        // got a host.  everybody skips localhost and 127.0.0.1,
        // doze groks //host/path
        if (!firstEdge.compare("localhost") || !firstEdge.compare("127.0.0.1")) {
            rval = Path("/");
        } else {
#ifdef WIN32
            rval = Path("//" + firstEdge);
#else
            return rval;
#endif
        }
        start++;
    }

    // add remaining edges
    for (size_t i = start; i < edges.size(); i++) {
        rval /= Path(urlDecode(edges[i]));
    }

    return rval;
}


static bool
unsetReadOnly(const Path& path)
{
    bool rval = false;
    FileInfo fi;
    if (statFile(path, fi)) {
        if ((fi.mode & 0200) == 0) {
            fi.mode |= 0200;
            if (setFileProperties(path, fi)) {
                rval = true;
            } else {
                BPLOG_DEBUG_STRM("setFileProperties(" << path << ") failed");
            }
        }
    } else {
        BPLOG_DEBUG_STRM("statFile(" << path << ") failed")
    }
    return rval;
}

    
bool 
remove(const Path& path)
{

    // easy, path doesn't exist
    if (path.empty() || !bfs::exists(path)) {
        return true; 
    }

    bool rval = false;
    try {
        bfs::remove_all(path);
        rval = true;
    } catch(const tFileSystemError&) {
        // bfs::remove_all can fail if anything is read-only.
        // Thus, we'll recursively remove the attribute and try again.
        BPLOG_DEBUG_STRM("bfs::remove_all(" << path
                         << ") failed, removing read-only attributes "
                         << "and trying again");
        if (bfs::is_directory(path)) {
            tRecursiveDirIter end;
            for (tRecursiveDirIter it(path); it != end; ++it) {
                Path p(it->path());
                (void) unsetReadOnly(p);
            }
        } else {
            (void) unsetReadOnly(path);
        }
        // Ability to delete p depends upons permissions on 
        // p's parent.  Try to give parent write permission,
        // then restore old permission when done.
        FileInfo parentInfo;
        Path parent = path.parent_path();
        bool setParentInfo = statFile(parent, parentInfo)
                             && unsetReadOnly(parent);
        try {
            bfs::remove_all(path);
            rval = true;
        } catch(const tFileSystemError& e) {
            BPLOG_WARN_STRM("bfs::remove_all(" << path
                            << ") failed: " << e.what());
        }
        if (setParentInfo) {
            setFileProperties(parent, parentInfo);
        }
    }
    return rval;
}


bool
move(const Path& from,
     const Path& to)
{
    try {
        bfs::rename(from, to);
    } catch(const tFileSystemError& e) {
        BPLOG_WARN_STRM("move(" << from
                        << ", " << to << ") failed: " << e.what()
                        << ", trying copy/delete");
        if (!copy(from, to)) {
            BPLOG_WARN_STRM("copy failed");
            return false;
        }
        if (!remove(from)) {
            BPLOG_WARN_STRM("delete after copy failed: " << from
                            << ", adding for delayed delete");
            s_delayDelete.insert(from);
        }
    }
    return true;
}


void
delayDelete()
{
    set<Path>::iterator it;
    for (it = s_delayDelete.begin(); it != s_delayDelete.end(); ++it) {
        Path p(*it);
        BPLOG_DEBUG_STRM("attempt to delete " << p);
        (void) remove(p);
    }
}


static void
copyDir(const Path& from,
        const Path& to)
{
    tString fromStr = from.string();
    tRecursiveDirIter end;
    for (tRecursiveDirIter it(from); it != end; ++it) {
        Path relPath = Path(it->path()).relativeTo(from);
        Path target = to / relPath;
        if (bfs::is_directory(it->path())) {
            bfs::create_directories(target);
        } else {
            bfs::copy_file(it->path(), target);
        }
    }
}


bool 
copy(const Path& src,
     const Path& dst,
     bool followLinks)
{
    try {
        // fail on bad args
        if (src.empty() || dst.empty()) {
            return false;
        }

        // fail if source doesn't exist
        if (!bfs::exists(src)) {
            return false;
        }
    
        // fail if destination file exists (no implicit overwrite)
        if (bfs::exists(dst) && !bfs::is_directory(dst)) {
            return false;
        }

        // chase links if asked, broken links cause failure
        Path from = src;
        if (followLinks && isLink(src)) {
            if (!resolveLink(src, from)) {
                return false;
            } 
        }
        Path to = dst;
        if (followLinks && isLink(dst)) {
            if (!resolveLink(dst, to)) {
                return false;
            } 
        }

        // trailing / on dirs ok, but not on files
        string fromStr = from.utf8();
        if (fromStr.rfind("/") == fromStr.length()-1) {
            if (bfs::is_directory(from)) {
                from = fromStr.substr(0, fromStr.length()-1);
            } else {
                return false;
            }
        }
        string toStr = to.utf8();
        if (toStr.rfind("/") == toStr.length()-1) {
            if (bfs::exists(to) && !bfs::is_directory(to)) {
                return false;
            }
            to = toStr.substr(0, toStr.length()-1);
        }

        if (bfs::is_directory(from)) {
            // source is a directory
            Path target = to;
            if (bfs::is_directory(to)) {
                // copy into dest, creating new dir with
                // basename of source
                target = to / from.filename();
                bfs::create_directory(target);
            } else if (bfs::is_directory(to.parent_path())) {
                // copy source to new dir, losing original basename
                bfs::create_directory(target);
            } else {
                // dest isn't a dir, nor is it's parent. fail
                return false;
            }
            copyDir(from, target);
        } else {
            // source is file
            if (src.utf8().rfind("/") == src.utf8().length() - 1) {
                // no trailing / allowed in src
                return false;
            }
            Path target = to;
            if (bfs::is_directory(to)) {
                // dest is directory, copy preserving filename
                target = to / from.filename();
            } else if (!bfs::is_directory(to.parent_path())) {
                // dest isn't a dir, nor is it's parent. fail
                return false;
            }
            bfs::copy_file(from, target);
        }
    } catch(tFileSystemError& e) {
        BPLOG_ERROR_STRM("copy(" << src << ", " << dst
                         << " failed: " << e.what());
        return false;
    }
    return true;
}


bool
openReadableStream(ifstream& fstream,
                   const Path& path,
                   int flags)
{
    if (fstream.is_open()) {
        BPLOG_WARN_STRM("openReadableStream, stream already open");
        return false;
    }
    tString native = path.external_file_string();
#ifdef WIN32
    fstream.open(native.c_str(), ios::in | flags);
#else
    fstream.open(native.c_str(), ios::in | (_Ios_Openmode) flags);
#endif
    if (!fstream.is_open()) {
        BPLOG_WARN_STRM("openReadableStream, stream open failed for " << path);
        return false;
    }
    return true;
}


bool
openWritableStream(std::ofstream& fstream,
                   const Path& path,
                   int flags)
{
    if (fstream.is_open()) {
        BPLOG_WARN_STRM("openWritableStream, stream already open");
        return false;
    }
    tString native = path.external_file_string();
#ifdef WIN32
    fstream.open(native.c_str(), std::ios::out | flags);
#else
    fstream.open(native.c_str(), std::ios::out | (_Ios_Openmode) flags);
#endif

	// set user read/write permission if needed
    tStat sb;
    if (::stat(native.c_str(), &sb) != 0) {
        BPLOG_WARN_STRM("openWritableStream, unable to stat " << path);
        return false;
    }
    if ((sb.st_mode & (S_IRUSR|S_IWUSR)) != (S_IRUSR|S_IWUSR)) {
        if (::chmod(native.c_str(), S_IRUSR | S_IWUSR) != 0) {
            BPLOG_WARN_STRM("openWritableStream, unable to chmod " << path);
            return false;
        }
    }
    if (!fstream.is_open()) {
        BPLOG_WARN_STRM("openWritableStream, stream open failed for " << path);
        return false;
    }
    return true;
}

bool
makeReadOnly(const Path& path)
{
    // toggle bits to make this thing read only
    FileInfo fi;
    if (!statFile(path, fi)) {
        return false;
    }
    
    // turn off write bits (making the file _read_only_)
    fi.mode &= ~(0222);

    if (!setFileProperties(path, fi)) {
        return false;
    }
    
    return true;
}


set<string> 
mimeTypes(const Path& p)
{
    initializeMimeTypes();
    set<std::string> rval;

    // deal with links
    Path target(p);
    if (isLink(p)) {
        if (resolveLink(p, target)) {
            rval.insert(kLinkMimeType);
        } else {
            rval.insert(kBadLinkMimeType);
        }
        return rval;
    }

    if (bfs::is_directory(target)) {
        rval.insert(kFolderMimeType);
        return rval;
    }

    // get extension, boost includes the .
    tString ext = target.extension();
    if (!ext.empty()) {
        ext = ext.substr(1, string::npos);
        TExtMap::const_iterator it = s_extensionMap.find(utf8FromNative(ext));
        if (it != s_extensionMap.end()) {
            rval = it->second;
        }
    }
    if (rval.empty()) {
        rval.insert("application/unknown");
    }
    return rval;
}


bool 
isMimeType(const Path& p,
           const set<std::string>& filter)
{
    if (filter.empty()) {
        return true;
    }
    set<std::string> myTypes = mimeTypes(p);
    set<std::string>::const_iterator it;
    for (it = myTypes.begin(); it != myTypes.end(); ++it) {
        if (filter.count(*it) > 0) {
            return true;
        }
    }
    return false;
}


vector<string>
extensionsFromMimeType(const string& mimeType)
{
    vector<string> rval;
    initializeMimeTypes();
    TExtMap::const_iterator it;
    for (it = s_extensionMap.begin(); it != s_extensionMap.end(); ++it) {
        if (it->second.count(mimeType)) {
            rval.push_back(it->first);
        }
    }
    sort(rval.begin(), rval.end());
    return rval;
}

}}

