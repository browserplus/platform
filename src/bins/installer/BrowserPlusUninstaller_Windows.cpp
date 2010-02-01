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
 * Portions created by Yahoo! are Copyright (c) 2009 Yahoo! Inc.
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
#include "BPUtils/BPFile.h"
#include "BPUtils/bplocalization.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpprocess.h"
#include "BPUtils/bpstrutil.h"

using namespace bp::file;
using namespace bp::localization;
using namespace bp::strutil;
using namespace std;


// Forward Declarations
void doWork( const vector<string>& vsArgs );
void forkChild( const string& sChildName, const vector<string>& vsArgs );



void setupLogging( bool bTruncateExisting )
{
    Path logFile = getTempDirectory() / "BrowserPlusUninstaller.log";
    string sLogLevel = "debug";
    
    bp::log::setupLogToFile( logFile, sLogLevel, bTruncateExisting );
}
    


int WINAPI WinMain( HINSTANCE, HINSTANCE, PSTR, int )
{
    vector<string> vsArgs = bp::process::getCommandLineArgs();
    
    // Parent has no command line args (currently).
    bool bParent = vsArgs.size() == 1;
    
    if (bParent)
    {
        setupLogging( true );
        BPLOG_INFO( "Inside parent uninstaller process." );

        // Fork a temporary self-deleting child with all our args.
        // That child will delete our exe and then do the real work.
        forkChild( "bpuninstall.exe", vsArgs );

        // Exit normally
        return 0;
    }
    else
    {
        setupLogging( false );
        BPLOG_INFO( "Inside child uninstaller process." );
        
        // We're the child, do whatever work we want to do.
        doWork( vsArgs );

        // Exit normally.
        // Child exe will be automatically deleted due to how it was created.
        return 0;
    }
}	


void forkChild( const string& sChildName, const vector<string>& vsParentArgs )
{
    // Get path to current (parent) exe.
    wchar_t wszParentPath[MAX_PATH];
    GetModuleFileNameW( NULL, wszParentPath, MAX_PATH );

    // Copy parent exe into temp file.
    Path childPath = getTempDirectory() / sChildName;
    CopyFileW( wszParentPath, childPath.external_file_string().c_str(), FALSE );

    // Open temp file with delete-on-close.
    HANDLE hChild = CreateFileW( childPath.external_file_string().c_str(),
                                 GENERIC_READ,
                                 FILE_SHARE_READ|FILE_SHARE_DELETE, 0,
                                 OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, 0 );

    // Setup the child's command line.
    vector<string> vsChildArgs;
    vsChildArgs.push_back( childPath.utf8() ); 

    // Add all the parent's args.
//  for (vector<string>::const_iterator it = vsParentArgs.begin();
//       it != vsParentArgs.end(); ++it) {
//        vwChildArgs.push_back( bp::strutil::utf8ToWide( *it ) );
//  }                               
    
    // Add quoted path to parent exe so child can delete it.
    Path parentPath(wszParentPath);
    vsChildArgs.push_back( "\"" + parentPath.utf8() + "\"" );

    // Convert arg vec to wide string.
    stringstream ss;
    copy( vsChildArgs.begin(), vsChildArgs.end(),
          std::ostream_iterator<string>( ss, " " ) );
    wstring wsChildArgs = utf8ToWide( ss.str() );
    
    // CreateProcess demands a mutable cmd line.
    wchar_t wszChildCmd[MAX_PATH];
    wcsncpy_s( wszChildCmd, wsChildArgs.c_str(), MAX_PATH );

    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(STARTUPINFOW));
    si.cb = sizeof(STARTUPINFOW);
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    CreateProcessW( 0, wszChildCmd, 0, 0, FALSE, NORMAL_PRIORITY_CLASS,
                    0, 0, &si, &pi );

    // Give the child a chance to run.
    Sleep( 100 );

    // Close our handle to the new process. Because the process is
    // memory-mapped, there will still be a handle held by the O/S, so
    // it won't get deleted.
    CloseHandle( hChild );
}


void doWork( const vector<string>& vsArgs )
{
    // Get the strings we need now, in case we need them after deleting
    // resource files.
    const string sLocale = getUsersLocale();

    string sBP;
    getLocalizedString( "productNameShort", sLocale, sBP );
    wstring wsBP = utf8ToWide( sBP );

    string sPrompt;
    getLocalizedString( "uninstallPrompt", sLocale, sPrompt );
    wstring wsPrompt = utf8ToWide( sPrompt );

    string sDone;
    getLocalizedString( "uninstallNotification", sLocale, sDone );
    wstring wsDone = utf8ToWide( sDone );

    // Prompt user for confirmation.
    int nRtn = MessageBoxW( NULL,
                            wsPrompt.c_str(), wsBP.c_str(),
                            MB_OKCANCEL|MB_DEFBUTTON2 );
    if (nRtn == IDCANCEL)
        return;

    // Give the parent process time to exit.  We want this
    // since "unins.run()" will try to remove the parent.
    Sleep( 200 );

    // Perform the actual uninstall.
    bp::install::Uninstaller unins;
    unins.run();

    // Inform user when done.
    MessageBoxW( NULL, wsDone.c_str() , wsBP.c_str(), MB_OK );
}

