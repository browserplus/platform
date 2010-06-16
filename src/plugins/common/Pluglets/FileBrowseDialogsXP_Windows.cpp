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

/*
 *  FileBrowseDialogsXP_Windows.cpp
 */

#include "FileBrowseDialogs_Windows.h"

#include <atlbase.h>
#include <atlcom.h>
#include <CommDlg.h>
#include <Dlgs.h>
#include <shlobj.h>
#include <shobjidl.h>


#include "BPUtils/bpfile.h"
#include "BPUtils/bplocalization.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpstrutil.h"
#include "FileBrowsePluglet.h"


using namespace std;
using namespace bp::strutil;
namespace bpf = bp::file;
namespace bpl = bp::localization;


// --------------------------- XP hooking junk

// The standard GetOpenFileNameW() dialog doesn't allow folder selection,
// and SHBrowseForFolder() is both ugly as sin and doesn't allow multiple selection.
// We add a hook and our own winproc to the dialog to allow multiple selection of 
// both files and folders. 

// GetOpenFileNameW control identifiers
// chx1 The read-only check box
// cmb1 Drop-down combo box that displays the list of file type filters
// stc2 Label for the cmb1 combo box
// cmb2 Drop-down combo box that displays the current drive or folder, and that allows the user to select a drive or folder to open
// stc4 Label for the cmb2 combo box
// cmb13    Drop-down combo box that displays the name of the current file, allows the user to type the name of a file to open, and select a file that has been opened or saved recently. This is for earlier Explorer-compatible applications without hook or dialog template. Compare with edt1.
// edt1 Edit control that displays the name of the current file, or allows the user to type the name of the file to open. Compare with cmb13.
// stc3 Label for the cmb13 combo box and the edt1 edit control
// lst1 List box that displays the contents of the current drive or folder
// stc1 Label for the lst1 list box
// IDOK The OK command button (push button)
// IDCANCEL The Cancel command button (push button)
// pshHelp  The Help command button (push button)

// A context passed into hook and then winproc
typedef struct {
    WNDPROC m_winProc;             // original dialog winproc
    vector<bpf::Path>* m_paths;    // selected files/folders
    string m_locale;
} Context;


// our winproc, figures out selection gunk
//
static LRESULT CALLBACK 
MyWinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Context* ctx = (Context*) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (uMsg == WM_COMMAND &&
          HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDOK)
    {
        // get dialog's list view
        HWND list = GetDlgItem(GetDlgItem(hWnd, lst2), 1);

        // is anything selected?
        bool valid = false;
        int idx = ListView_GetNextItem(list, -1, LVNI_SELECTED);
        if (idx == -1) {
            // Nothing selected. If the combo box is not empty, use it.
            // Otherwise, use current folder.
            TCHAR buf[MAX_PATH];
            SendMessage(hWnd, CDM_GETFOLDERPATH, MAX_PATH, (LPARAM)buf);
            wstring wstr((wchar_t*)buf);

            UINT nchars = GetDlgItemText(hWnd, cmb13, buf, MAX_PATH);
            wstring comboText;
            if (nchars > 0) {
                comboText = (wchar_t*) buf;

                // Trim leading/trailing spaces, which are invalid.
                comboText = utf8ToWide(trim(wideToUtf8(comboText)));
            }

            if (comboText.length()) {
                // path in buf may be fully-qualified or relative
                // to current folder. Presence of ":\" is the indicator
                // of an absolute uri
                if (comboText.find(L":\\") == 1) {
                    wstr = comboText;
                } else {
                    wstr = wstr + L"\\" + comboText;
                }

                bpf::Path path(wstr);
                if (bpf::exists(path)) {
                    ctx->m_paths->push_back(path);
                    valid = true;
                }
            }
        } else {
            // Stuff is selected, let's go get it.  LVITEM.lParam is a pidl, from which we
            // get the full path (see http://msdn.microsoft.com/msdnmag/issues/03/09/CQA/
            while (idx > -1) {
                LVITEM item = {LVIF_PARAM, idx, 0};
                ListView_GetItem(list, &item);

                // item.lParam is a PIDL.  First get PIDL of current folder 
                // (get length first, then allocate)
                int len = SendMessage(hWnd, CDM_GETFOLDERIDLIST, 0, NULL);
                if (len > 0) {
                    LPCITEMIDLIST pidlFolder = (LPCITEMIDLIST)::CoTaskMemAlloc(len);
                    if (pidlFolder == NULL) {
                        BPLOG_ERROR("CoTaskMemAlloc failed");
                        return TRUE;
                    }
                    SendMessage(hWnd, CDM_GETFOLDERIDLIST, len, (LPARAM)pidlFolder);

                    // Now get IShellFolder for pidlFolder
                    CComQIPtr<IShellFolder> pDesktop;
                    SHGetDesktopFolder(&pDesktop);
                    CComQIPtr<IShellFolder> pFolder;
                    HRESULT hr = pDesktop->BindToObject(pidlFolder, NULL, 
                        IID_IShellFolder,
                        (void**)&pFolder);
                    if (!SUCCEEDED(hr)) {
                        pFolder = pDesktop;
                    }

                    // Finally, get display name
                    STRRET str = {STRRET_WSTR};
                    hr = pFolder->GetDisplayNameOf((LPITEMIDLIST)item.lParam, 
                        SHGDN_NORMAL | SHGDN_FORPARSING,
                        &str);
                    if (SUCCEEDED(hr)) {
                        bpf::Path path(str.pOleStr);
                        if (bpf::exists(path)) {
                            ctx->m_paths->push_back(path);
                            valid = true;
                        }
                        ::CoTaskMemFree(str.pOleStr);
                    }
                    ::CoTaskMemFree((void*)pidlFolder);
                }
                idx = ListView_GetNextItem(list, idx, LVNI_SELECTED);
            } // while have selected items
        } // else items were selected

        if (valid) {
            // we handled it
            EndDialog(hWnd, IDOK);
            return TRUE;
        }
    } // if WM_COMMAND, BN_CLICKED, IDOK

    // let the original winproc handle the message
    return CallWindowProc(ctx->m_winProc, hWnd, uMsg, wParam, lParam);
}


// hook to setup our winproc and change a of couple labels
//
static UINT APIENTRY 
MyHook(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG) {
        OPENFILENAME* ofn = (OPENFILENAME*) lParam;
        Context* ctx = (Context*) ofn->lCustData;

        // The actual dialog is our parent.
        HWND hWnd = GetParent(hDlg);

        // Set the text for the "Select" button.
        string lstr;
        (void) bpl::getLocalizedString(FileBrowsePluglet::kSelectKey,
                                       ctx->m_locale, lstr);
        wstring wlstr = bp::strutil::utf8ToWide(lstr);
        SendMessage(hWnd, CDM_SETCONTROLTEXT, IDOK, (LPARAM)(wchar_t*)wlstr.c_str());

        // Set the label for the combo box.
        (void) bpl::getLocalizedString(FileBrowsePluglet::kFileFolderNameKey,
                                       ctx->m_locale, lstr);
        wlstr = bp::strutil::utf8ToWide(lstr);
        SendMessage(hWnd, CDM_SETCONTROLTEXT, stc3, (LPARAM)(wchar_t*)wlstr.c_str());

        // Setup the dialog to use our custom wnd proc.
        ctx->m_winProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, 
            (LONG_PTR)MyWinProc);
        SetLastError(0);
        LONG_PTR prev = SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) ctx);
        if (prev == 0 && GetLastError() != 0) {
            BPLOG_ERROR("SetWindowLongPtr(...GWLP_USERDATA...) failed!");
        }

        return TRUE;
    }

    // Handle the CDN_FILEOK notification.
    // We are sent this to allow us to accept/reject file selection,
    // *prior* to the dialog handling this.
    // Turns out though that we really don't have enough information to
    // answer the question, and we forward the message to the dialog to
    // resolve the issue.
    //
    // There are 3 desired dialog exit conditions:
    // 1) The user explicitly clicks the Select button.
    // 2) The user hits enter when the Select button has input focus.
    // 3) The user double-clicks a leaf (non-folder) file.
    // 
    // Case 1: We do not get a notification, our win proc does.
    // Case 2: We do get a notification, and since we do
    //         DWL_MSGRESULT=TRUE below, the dialog does not exit.
    //         The win proc would have received an IDOK click even without our
    //         PostMessage, which is redundant in this case but harmless.
    // Case 3: We do get a notification, and since we do
    //         DWL_MSGRESULT=TRUE below, the dialog does not exit, but
    //         it turns out no notification message is sent to our win proc.
    //         (YIB-2915405).
    //         The PostMessage causes a double-click to be treated like
    //         a select followed by a click on the Select button.
    //         Note that a CDN_FILEOK is not sent for double-clicks on
    //         folders in the list box, instead a drilldown is
    //         performed, which is perfect.
    if (uMsg == WM_NOTIFY && ((NMHDR*)lParam)->code == CDN_FILEOK)
    {
        PostMessage(GetParent(hDlg), WM_COMMAND,
                    MAKEWPARAM(IDOK, BN_CLICKED), 0);

        // Say that we (the hook) don't want to allow/cause exit.
        // We'll let that be handled by dialog's OnOK handler.
        SetWindowLongPtr(hDlg, DWL_MSGRESULT, TRUE);
        return TRUE;
    }    

    return FALSE;
}


bool runFileOpenDialogXP(const FileOpenDialogParms& parms,
                         vector<bpf::Path>& vPaths)
{
    // Setup the openfilename structure.
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);

    // set this to browser hwnd.
    ofn.hwndOwner = (HWND) parms.parentWnd;

    // Set lpstrFile[0] to '\0' so that GetOpenFileNameW does not 
    // use the contents of szFile to initialize itself.
    wchar_t szFile[32768];
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);

    wstring filter(parms.filterName);
    filter.append(1, '\0');
    filter.append(parms.filterPattern);
    filter.append(2, '\0');
    ofn.lpstrFilter = (wchar_t*) filter.c_str();
    ofn.nFilterIndex = 1;

    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrDefExt = NULL;

    ofn.lpstrTitle = parms.title.c_str();

    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_ENABLESIZING;
    if (!parms.chaseLinks) {
        ofn.Flags |= OFN_NODEREFERENCELINKS;
    }

    // Our hook allows folder selection.  vPaths
    // populated by MyWinProc();
    ofn.Flags |= OFN_ENABLEHOOK;
    ofn.lpfnHook = MyHook;
    Context context = { NULL, &vPaths, parms.locale };
    ofn.lCustData = (LPARAM) &context;

    // Display the open dialog box, which populates vPaths.
    BOOL bStat = GetOpenFileNameW(&ofn);
    if (!bStat && CommDlgExtendedError()) {
        BPLOG_ERROR("GetOpenFileNameW failed.");;
        return false;
    }

    // ofn.lpstrFile only seems to be set for a double-clicking
    // a single item (in spite of what msdn claims).  In the case
    // of double-clicking broken shortcuts which were "fixed", 
    // it contains the fixed path, so use it.
    bpf::Path selPath(ofn.lpstrFile);
    if (!selPath.empty() && vPaths.size() == 1) {
        vPaths[0] = selPath;
    }

    return true;
}

