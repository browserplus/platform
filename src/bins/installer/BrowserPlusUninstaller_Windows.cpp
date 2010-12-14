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


// BrowserPlus Windows Uninstaller
// See: http://www.catch22.net/tuts/selfdel for info on exe self-delete.

#include <sstream>
#include <string>
#include <windows.h>
#include "BPInstaller/BPInstaller.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpprocess.h"
#include "BPUtils/bpstrutil.h"
#include "platform_utils/bpexitcodes.h"
#include "platform_utils/bplocalization.h"
#include "platform_utils/ProductPaths.h"

using namespace bp::file;
using namespace bp::localization;
using namespace bp::strutil;
using namespace std;
namespace bfs = boost::filesystem;


// Forward Declarations
int doChild( const vector<string>& vsArgs );
int doParent( const vector<string>& vsArgs );
int doWork( const vector<string>& vsArgs );
void forkChild( const string& sChildName, const vector<string>& vsArgs );



void setupLogging( bool bIsParent )
{
    bfs::path logFile = getTempDirectory().parent_path() / "BrowserPlusUninstaller.log";
    bp::log::Level level = bp::log::LEVEL_DEBUG;

    bp::log::FileMode mode = bIsParent ?
                             bp::log::kSizeRollover : bp::log::kAppend;

    bp::log::setLogLevel( level );
    bp::log::setupLogToFile( logFile, level, mode );
}
    


int WINAPI WinMain( HINSTANCE, HINSTANCE, PSTR, int )
{
    try
    {
        vector<string> vsArgs = bp::process::getCommandLineArgs();
        vsArgs.erase( vsArgs.begin() );  // strip command name

        // child has /child argument
        bool bIsChild = false;
        for (vector<string>::const_iterator it = vsArgs.begin();
             it != vsArgs.end(); ++it) {
            if (it->compare( "/child" ) == 0) {
                bIsChild = true;
                break;
            }
        }
        return bIsChild ? doChild( vsArgs ) : doParent( vsArgs );
    }
    catch (const std::exception& exc) {
        BP_REPORTCATCH(exc);
        return bp::exit::kCaughtException;
    }
    catch (...) {
        BP_REPORTCATCH_UNKNOWN;
        return bp::exit::kCaughtException;
    }
}	


int doParent( const vector<string>& vsArgs )
{
    setupLogging( true );
    BPLOG_FUNC_SCOPE;

    // Fork a temporary self-deleting child with all our args.
    // That child will delete our exe and then do the real work.
    forkChild( "bpuninstall.exe", vsArgs );

    return bp::exit::kOk;
}


int doChild( const vector<string>& vsArgs )
{
    setupLogging( false );
    BPLOG_FUNC_SCOPE;

    // We're the child, do whatever work we want to do.
    return doWork( vsArgs );
}


void forkChild( const string& sChildName, const vector<string>& vsParentArgs )
{
    BPLOG_FUNC_SCOPE;

    // Get path to current (parent) exe.
    wchar_t wszParentPath[MAX_PATH];
    GetModuleFileNameW( NULL, wszParentPath, MAX_PATH );

    // Copy parent exe into temp file in system temp dir.
    bfs::path tempDir = getTempDirectory().parent_path();
    HANDLE hChild = NULL;
    bfs::path childPath;
    vector<string> vsChildArgs;
    wchar_t* wszChildCmd = NULL;
    try {
        childPath = tempDir / sChildName;

        // Setup the child's command line.
        vsChildArgs.push_back( "\"" + nativeUtf8String(childPath) + "\""); 
        vsChildArgs.push_back("/child");

        // Add all the parent's args.
        for (vector<string>::const_iterator it = vsParentArgs.begin();
             it != vsParentArgs.end(); ++it) {
            vsChildArgs.push_back( *it );
        }

        // Create a deleteOnClose copy of ourselves and launch it
        CopyFileW( wszParentPath, nativeString(childPath).c_str(), FALSE );

        // Open temp file with delete-on-close.
        hChild = CreateFileW( nativeString(childPath).c_str(),
                              GENERIC_READ,
                              FILE_SHARE_READ|FILE_SHARE_DELETE, 0,
                              OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, 0 );
        if ( !hChild ) {
            throw string( "unable to create handle for " + childPath.string() );
        }

        // Convert arg vec to wide string.
        stringstream ss;
        copy( vsChildArgs.begin(), vsChildArgs.end(),
              std::ostream_iterator<string>( ss, " " ) );
        wstring wsChildArgs = utf8ToWide( ss.str() );
    
        // CreateProcess demands a mutable cmd line. 
        size_t len = wsChildArgs.length() + 1;
        wszChildCmd = new wchar_t[len];
        wcsncpy_s( wszChildCmd, len, wsChildArgs.c_str(), len-1 );

        STARTUPINFOW si;
        ZeroMemory(&si, sizeof(STARTUPINFOW));
        si.cb = sizeof(STARTUPINFOW);
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        BOOL bRet = CreateProcessW( 0, wszChildCmd, 0, 0, FALSE, 
                                    NORMAL_PRIORITY_CLASS, 0, 0, &si, &pi );
        if ( !bRet ) {
            throw string( bp::error::lastErrorString( "failed to launch: " 
                                                      + wideToUtf8( wszChildCmd )));
        }

        // Give the child a chance to run.
        BPLOG_INFO_STRM( "launched: " << wideToUtf8( wszChildCmd ));
        Sleep( 100 );
    } catch (const string& s ) {
        // hrm, couldn't run child.  We'll do the uninstall ourselves.  
        // This may leave cruft, especially BrowserPlusUninstaller.exe, 
        // but it's the best we can do.
        BPLOG_ERROR( "error launching child uninstall (\'" + s 
                     + "\'), completing uninstall in parent" );
        safeRemove( childPath );
        doWork( vsChildArgs );
    }

    if ( wszChildCmd ) {
        delete[] wszChildCmd;
    }

    // Close our handle to the new process. Because the process is
    // memory-mapped, there will still be a handle held by the O/S, so
    // it won't get deleted.
    if ( hChild ) {
        CloseHandle( hChild );
    }
}


int doWork( const vector<string>& vsArgs )
{
    BPLOG_FUNC_SCOPE;
    
    // if we have a /quiet or -quiet arg, no dialogs
    bool bQuiet = false;
    for (vector<string>::const_iterator it = vsArgs.begin();
         it != vsArgs.end(); ++it) {
        if (it->compare( "/quiet" ) == 0 || it->compare( "-quiet" ) == 0) {
            bQuiet = true;
            break;
        }
    }

    wstring wsBP, wsPrompt, wsDone;
    if (!bQuiet) {
        // Get the strings we need now, in case we need them after deleting
        // resource files.
        const string sLocale = getUsersLocale();

        string sBP;
        getLocalizedString( "productNameShort", sLocale, sBP );
        wsBP = utf8ToWide( sBP );

        string sPrompt;
        getLocalizedString( "uninstallPrompt", sLocale, sPrompt );
        wsPrompt = utf8ToWide( sPrompt );

        string sDone;
        getLocalizedString( "uninstallNotification", sLocale, sDone );
        wsDone = utf8ToWide( sDone );

        // Prompt user for confirmation.
        BPLOG_INFO( "Displaying uninstall prompt." );
        int nRtn = MessageBoxW( NULL,
                                wsPrompt.c_str(), wsBP.c_str(),
                                MB_OKCANCEL|MB_DEFBUTTON2 );
        if (nRtn == IDCANCEL) {
            BPLOG_INFO( "User cancelled uninstall" );
            return bp::exit::kUserCancel;
        }
    }

    // Give the parent process time to exit.  We want this
    // since "unins.run()" will try to remove the parent.
    Sleep( 200 );

    // Perform the actual uninstall.
    BPLOG_INFO( "Invoking Uninstaller::run()." );
    bfs::path logFile = getTempDirectory().parent_path() / "BrowserPlusUninstaller.log";
    bp::log::Level level = bp::log::LEVEL_DEBUG;
    bp::install::Uninstaller unins(logFile, level);
    unins.run();

    if (!bQuiet) {
        // Inform user when done.
        BPLOG_INFO( "Displaying uninstall notification." );
        MessageBoxW( NULL, wsDone.c_str() , wsBP.c_str(), MB_OK );
    }

    return bp::exit::kOk;
}

