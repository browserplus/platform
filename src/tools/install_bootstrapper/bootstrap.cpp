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
 * Portions created by Yahoo! are Copyright (c) 2010 Yahoo! Inc.
 * All rights reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/**
 * boostrapper
 *
 * Originally Authored by Lloyd Hilaiel on Feb 6th 2009
 * (c) Yahoo! 2009
 */

#include <atlbase.h>
#include <atlwin.h>
#include <fstream>
#include <iostream>
#include "ArchiveLib/ArchiveLib.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpprocess.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bperrorutil.h"

#define BS_INFO_OUTPUT( x ) BPLOG_INFO_STRM(x)
#define BS_ERROR_OUTPUT( x ) BPLOG_ERROR_STRM(x)

bp::file::Path g_exePath;
const static std::string endOfPayloadMarker(
    "Yahoo!BrowserPlusEndOfInstallerPayloadMarker");


// Given that this is a gui app, we don't provide "usage" behavior.
// Supported cmd-line args documented here:
//   [-logfile=<filename>|console]
//     override default log file and log to specified file or console
//     this is also passed to spawned installer
//   [-pause=<anything>]
//     prompt for a keystroke at end of installation process
//     only supported when logfile=console
//   all other cmd-line args passed to spawned installer


int APIENTRY WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
    bool pauseAtEnd = false;
    
    // Scan argv for logfile flag and build up a vector of args we'll
    // relay into the spawned installer.  We always log, but 
    // different logfile can be specified via command line.
    // debug logging on be default
    std::vector<std::string> args;
    bp::file::Path logFile = bp::file::getTempDirectory()/"BrowserPlusInstaller.log";

    for (int i = 1; i < __argc; i++) {
        std::vector<std::string> arg = bp::strutil::split(__argv[i], "=");
        if (arg.size() == 2 && !arg[0].compare("-logfile")) {
            if (!arg[1].compare("console")) {
                logFile.clear();
            } else {
                logFile = arg[1];
            }
        } else if (!arg[0].compare("-pause")) {
            pauseAtEnd = true;
        }
        else {
            args.push_back(std::string(__argv[i]));
        }
    }

    if (!logFile.empty()) {
        // Both the "bootstrap" process and the spawned "installer" process
        // may write to the same file.
        // bootstrap will be master and handle rollover, spawned
        // installer will just append.
        bp::log::setupLogToFile(logFile, bp::log::LEVEL_DEBUG, 
                                bp::log::kSizeRollover);
    } else {
        bp::log::setupLogToConsole(bp::log::LEVEL_DEBUG);
    }

    std::string logArg("-logfile=");
    logArg.append(logFile.utf8());
    args.push_back(logArg);

    wchar_t szPath[_MAX_PATH];
    GetModuleFileNameW( NULL, szPath, sizeof(szPath)/sizeof(wchar_t) );
    g_exePath = szPath;
    BS_INFO_OUTPUT( "Path to binary:" << g_exePath );

    // now let's find the embedded payload
    std::ifstream f;
    if (!bp::file::openReadableStream(f, g_exePath,
                                      std::ios_base::binary |
                                      std::ios_base::in))
    {
        BS_ERROR_OUTPUT("Couldn't open binary for reading.");
        return 1;
    }

    // reverse scan from eof looking for end of payload marker.
    {
        int endOff = endOfPayloadMarker.length();
        char buf[64];
        memset(buf, 0, sizeof(buf));
        BPASSERT(sizeof(buf) > endOfPayloadMarker.length());
        for (;;) {
            f.seekg(-endOff, std::ios_base::end);
            if (f.fail()) break;
            f.read(buf, endOfPayloadMarker.length());            
            if (f.fail()) break;
            if (!std::string(buf).compare(endOfPayloadMarker)) {
                break;
            }
            endOff++;
        }
        f.seekg(-endOff, std::ios_base::end);

        // how'd we do?
        if (f.fail()) {
            BS_ERROR_OUTPUT( "Couldn't find embedded payload, "
                             "installation failed" );
            return 1;
        }
        BS_INFO_OUTPUT( "Located installer payload " << endOff
                        << " bytes from EOF" );
    }

    // now let's seek to the end and read the last four bytes
    f.seekg(-4, std::ios_base::cur);
    int off = f.tellg();
    int sz;
    f.read((char *) &sz, sizeof(sz));
    BS_INFO_OUTPUT( sz << " compressed bytes embedded in installer.");

    f.seekg(-sz-4, std::ios::cur);
    off = f.tellg();
    BS_INFO_OUTPUT( "Extracting LZMA compressed data at offset: " << off );

    // now we're ready to unlzma the thing!
    std::stringstream out;
    {
        bp::lzma::Decompress d;
        d.setInputStream(f);
        d.setOutputStream(out);        
        if (!d.run(bp::lzma::LZIP))
        {
            BS_ERROR_OUTPUT( "Error decompressing LZMA data.");
            return 1;
        }

        BS_INFO_OUTPUT( "Main installer decompressed to " <<
                         out.str().length() << " bytes." );
    }

    // out now contains a tarball that we'll want to decompress to a
    // temporary directory
    bp::file::Path extractTo = bp::file::getTempPath(bp::file::getTempDirectory(), "BPINST");

    try {
        boost::filesystem::create_directories(extractTo);
    } catch(const bp::file::tFileSystemError&) {
        BS_ERROR_OUTPUT( "Couldn't create temporary directory: " << extractTo );
        return 1;
    }

    // now do the extraction
    BS_INFO_OUTPUT( "Extracting installer to " << extractTo );
    {
        bp::tar::Extract e;
        if (!e.load(out.str()))
        {
            BS_ERROR_OUTPUT( "Error initializing tar extraction." );
            return 1;
        }
        
        if (!e.extract(extractTo))
        {
            BS_ERROR_OUTPUT( "Error extracting installer data.");
            return 1;
        }

        (void) e.close();
    }

    // now spawn the installer
    {
        bp::file::Path instExe = extractTo / "BrowserPlusInstaller.exe";

        bp::process::spawnStatus status;

        // (relay arguments through to spawned process)
        if (!bp::process::spawn(instExe, "", extractTo,
                                args, &status))
        {
            BS_ERROR_OUTPUT( "Couldn't spawn installer executable: " << instExe );
            return 1;
        }

        // wait for our child
        int exitCode = 0;
        if (!bp::process::wait(status, true, exitCode))
        {
            BS_ERROR_OUTPUT( "Couldn't wait for child.");
            return 1;
        }

        if (exitCode != 0)
        {
            BS_ERROR_OUTPUT(
                "BrowserPlusInstaller returns non-zero exit code: " <<
                exitCode );
            return 1;
        }

        BS_INFO_OUTPUT( "BrowserPlusInstaller exits successfully." );
    }

    // now let's clean up.
    if (!bp::file::remove(extractTo))
    {
        BS_ERROR_OUTPUT( "Couldn't clean up temporary directory " << extractTo );
        return 1;
    }

    BS_INFO_OUTPUT( "Removed temporary directory: " << extractTo );

    // Support pause before killing spawned console.
    if (logFile.empty() && pauseAtEnd) {
        HANDLE hOut = GetStdHandle( STD_OUTPUT_HANDLE );
        std::wstring ws = bp::strutil::utf8ToWide("Press ENTER to continue...");
        DWORD numWritten = 0;
        WriteConsoleW( hOut, (void *) ws.c_str(), ws.length(),
                       &numWritten, NULL);

        HANDLE hIn = GetStdHandle( STD_INPUT_HANDLE );
        char in[80];
        DWORD numRead;
        ReadConsole( hIn, in, 1, &numRead, NULL );
    }
    
    return 0;
}

