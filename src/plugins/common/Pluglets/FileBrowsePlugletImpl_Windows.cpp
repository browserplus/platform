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

/*
 *  FileBrowsePlugletImpl_Windows.cpp
 *
 *  Created by David Grigsby on 11/20/2007.
 *  Modified to support multiple selects
 *  and mimetype filtering by Gordon Durand on 07/10/08.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 */

#define _ATL_APARTMENT_THREADED
#include <atlbase.h>
#include <atlcom.h>
#include <CommDlg.h>
#include <Dlgs.h>

#include "FileBrowsePluglet.h"

#include <vector>
#include <string>
#include <shlobj.h>
#include <shobjidl.h>

#include "bppluginutil.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bplocalization.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpurl.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/OS.h"
#include "CommonErrors.h"

using namespace std;
using namespace bp::localization;
using namespace bp::strutil;
namespace bpf = bp::file;

static wstring getBrowseTitle( const std::string& sUrl,
                               const std::string& sLocale )
{
    std::string sTitle;
    getLocalizedString( FileBrowsePluglet::kSelectFilesFoldersKey, sLocale,
                        sTitle );

    // For security, append the host name.
    bp::url::Url url;
    if (url.parse( sUrl ))
    {
        sTitle += " (";
        sTitle += url.friendlyHostPortString();
        sTitle += ")";
    }

    return bp::strutil::utf8ToWide( sTitle );
}


// --------------------------- Vista custom control event handling

// We add a "Select" button which allows selecting a single folder.
// Alas, multiple folder or file and folder selection doesn't work.


static void
getVistaSelection(IFileOpenDialog* pDlg,
                  vector<bpf::Path>* paths)
{
    paths->clear();
    if (!pDlg) {
        return;
    }
    CComPtr<IShellItemArray> pItemArray;
    HRESULT hr = pDlg->GetSelectedItems(&pItemArray);
    if (FAILED(hr)) {
        return;
    }
    DWORD count = 0;
    hr = pItemArray->GetCount(&count);
    if (FAILED(hr)) {
        return;
    }
    for (DWORD i = 0; i < count; i++) {
        CComPtr<IShellItem> pItem;
        hr = pItemArray->GetItemAt(i, &pItem);
        if (FAILED(hr)) {
            continue;
        }
        wstring wstr;
        LPOLESTR pwsz = NULL;
        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz);
        if (SUCCEEDED(hr)) {
            wstr.append(pwsz);
        }
        CoTaskMemFree(pwsz);
        bpf::Path path(wstr);
        if (!path.empty()) {
            paths->push_back(path);
        }
    }
}


class MyEventHandler 
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<MyEventHandler>,
      public IFileDialogEvents,
      public IFileDialogControlEvents
{
public:
    static const DWORD kSelectButtonId = 1000;

    MyEventHandler()
    {
    }

    ~MyEventHandler()
    {
    }

    BEGIN_COM_MAP(MyEventHandler)
        COM_INTERFACE_ENTRY(IFileDialogEvents)
        COM_INTERFACE_ENTRY(IFileDialogControlEvents)
    END_COM_MAP()

    // IFileDialogEvents
    STDMETHODIMP OnFileOk(IFileDialog* pfd) 
    {
        CComQIPtr<IFileOpenDialog> pDlg = pfd;
        getVistaSelection(pDlg, m_paths);
        return S_OK;
    }

    STDMETHODIMP OnFolderChanging(IFileDialog*, IShellItem*) 
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP OnFolderChange(IFileDialog*) 
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP OnSelectionChange(IFileDialog*)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP OnShareViolation(IFileDialog*, IShellItem*, 
                                  FDE_SHAREVIOLATION_RESPONSE*)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP OnTypeChange(IFileDialog*)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP OnOverwrite(IFileDialog*, IShellItem*, 
                             FDE_OVERWRITE_RESPONSE*) 
    {
        return E_NOTIMPL;
    }

    // IFileDialogControlEvents
    STDMETHODIMP OnItemSelected(IFileDialogCustomize*, 
                                DWORD, 
                                DWORD)
    {
        return S_OK;
    }

    STDMETHODIMP OnButtonClicked(IFileDialogCustomize* pfdc, 
                                 DWORD dwIDCtl)
    {
        if (dwIDCtl == kSelectButtonId) {
            CComQIPtr<IFileOpenDialog> pDlg = pfdc;
            getVistaSelection(pDlg, m_paths);
            pDlg->Close(S_OK);
        }
        return S_OK;
    }

    STDMETHODIMP OnCheckButtonToggled(IFileDialogCustomize*, 
                                      DWORD, 
                                      BOOL)
    {
        return S_OK;
    }

    STDMETHODIMP OnControlActivating(IFileDialogCustomize*, 
                                     DWORD)
    {
        return S_OK;
    }

    void setPaths(vector<bpf::Path>* paths)
    {
        m_paths = paths;
    }

private:
    vector<bpf::Path>* m_paths;
};


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
    WNDPROC m_winProc;                  // original dialog winproc
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
        wstring wlstr;
        (void) getLocalizedString(FileBrowsePluglet::kSelectKey, ctx->m_locale, lstr);
        wlstr = bp::strutil::utf8ToWide(lstr);
        SendMessage(hWnd, CDM_SETCONTROLTEXT, IDOK, (LPARAM)(wchar_t*)wlstr.c_str());

        // Set the label for the combo box.
        (void) getLocalizedString(FileBrowsePluglet::kFileFolderNameKey,
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


// ----------- FileBrowsePluglet impl

void
FileBrowsePluglet::execute(unsigned int tid,
                           const char* function,
                           const bp::Object* arguments,
                           bool /* syncInvocation */, 
                           plugletExecutionSuccessCB successCB,
                           plugletExecutionFailureCB failureCB,
                           plugletInvokeCallbackCB   /*callbackCB*/,
                           void* callbackArgument )
{
    if (!function) {
        BPLOG_WARN_STRM("execute called will NULL function");
        failureCB(callbackArgument, tid, pluginerrors::InvalidParameters, NULL);
        return;
    }

    if (strcmp(function, "OpenBrowseDialog")) {
        std::string s("unknown FileBrowse function " 
                      + std::string(function) + " called");
        failureCB(callbackArgument, tid, pluginerrors::InvalidParameters,
                  s.c_str());
        return;
    }

    bool recurse = true;
    std::set<std::string> mimetypes;
    bool includeGestureInfo = false;
    unsigned int limit = 1000;
    wstring filterName;
    wstring filterPattern;
    if (m_desc.majorVersion() == 1) {
        if (!arguments) {
            BPLOG_WARN_STRM("execute called will NULL arguments");
            failureCB(callbackArgument, tid, pluginerrors::InvalidParameters, NULL);
            return;
        }

        if (arguments->has("recurse", BPTBoolean)) {
            recurse = ((bp::Bool*) arguments->get("recurse"))->value();
        }
    
        if (arguments->has("mimeTypes", BPTList)) {
            const bp::List* l = (const bp::List*) arguments->get("mimeTypes");
            for (unsigned int i = 0; l && (i < l->size()); i++) {
                const bp::String* s = dynamic_cast<const bp::String*>(l->value(i));
                if (s) {
                    mimetypes.insert(s->value());
                }
            }
        } 
    
        if (arguments->has("includeGestureInfo", BPTBoolean)) {
            includeGestureInfo = ((bp::Bool*) arguments->get("includeGestureInfo"))->value();
        }
    
        if (arguments->has("limit", BPTInteger)) {
            limit = (unsigned int)(((bp::Integer*) arguments->get("limit"))->value());
        }

        // Get extensions for mimetypes and set filter from them
        if (mimetypes.empty()) {
            string lstr;
            (void) getLocalizedString(FileBrowsePluglet::kAllFilesFoldersKey,
                                      m_locale, lstr);
            filterName = bp::strutil::utf8ToWide(lstr);
            filterPattern.append(L"*.*");
        } else {
            set<string> extensions;
            set<string>::const_iterator iter;
            for (iter = mimetypes.begin(); iter != mimetypes.end(); ++iter) {
                filterName.append(bp::strutil::utf8ToWide(*iter));
                filterName.append(L", ");
                vector<string> e = bpf::extensionsFromMimeType(*iter);
                for (unsigned int i  = 0; i < e.size(); ++i) {
                    const string& s = e[i];
                    extensions.insert(s);
                }
            }
            filterName.erase(filterName.length()-2);  // nuke trailing ', '
            if (!extensions.empty()) {
                for (iter = extensions.begin(); iter != extensions.end(); ++iter) {
                    filterPattern.append(L"*.");
                    filterPattern.append(bp::strutil::utf8ToWide(*iter));
                    filterPattern.append(1, L';');
                }
                filterPattern.erase(filterPattern.length()-1);  // nuke trailing ;
            } else {
                filterPattern.append(L"*.*");
            }
        }
    }

    // selection ends up here
    vector<bpf::Path> vPaths;

    // different dialogs for xp and vista
    string osVersion = bp::os::PlatformVersion();
    bool isVistaOrLater = osVersion.compare("6.") > 0;

    if (isVistaOrLater) {
        CComPtr<IFileOpenDialog> pDlg;
        COMDLG_FILTERSPEC filters[] = {
            filterName.c_str(),
            filterPattern.c_str()
        };
        HRESULT hr = pDlg.CoCreateInstance(__uuidof(FileOpenDialog));
        if (FAILED(hr)) {
            failureCB(callbackArgument, tid, "FileBrowse.error",
                      "unable to create FileOpenDialog");
            return;
        }
        
        std::string sUrl;
        m_plugin->getCurrentURL(sUrl);
        wstring wsTitle = getBrowseTitle(sUrl, m_locale);
        pDlg->SetTitle(wsTitle.c_str());
        
        pDlg->SetFileTypes(_countof(filters), filters);
        DWORD flags = 0;
        pDlg->GetOptions(&flags);
        flags |= FOS_ALLOWMULTISELECT | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST
                 | FOS_FILEMUSTEXIST | FOS_DONTADDTORECENT;
        if (m_desc.majorVersion() > 1) {
            flags |= FOS_NODEREFERENCELINKS;
        }

        pDlg->SetOptions(flags);

        // add custom controls
        CComQIPtr<IFileDialogCustomize> pfdc = pDlg;
        if (!pfdc) {
            failureCB(callbackArgument, tid, "FileBrowse.error",
                      "unable to get IFileDialogCustomize ptr (should not happen!)");
            return;
        }
        std::string lstr;
        (void) getLocalizedString(FileBrowsePluglet::kSelectKey,
                                  m_locale, lstr);
        std::wstring wstr;
        wstr = bp::strutil::utf8ToWide(lstr);
        hr = pfdc->AddPushButton(MyEventHandler::kSelectButtonId,
                                (wchar_t*) wstr.c_str());
        if (FAILED(hr)) {
            failureCB(callbackArgument, tid, "FileBrowse.error",
                      "unable to add Select button to dialog");
            return;
        }

        // add event listener
        CComObjectStackEx<MyEventHandler> evh;
        evh.setPaths(&vPaths);
        CComQIPtr<IFileDialogEvents> pEvents = evh.GetUnknown();
        DWORD cookie;
        hr = pDlg->Advise(pEvents, &cookie);
        bool advised = SUCCEEDED(hr);

        // show dialog.  vPaths populated
        // by our event listener
        hr = pDlg->Show((HWND) m_plugin->getWindow());
        if (advised) {
            pDlg->Unadvise(cookie);
        }

    } else {
        // Setup the openfilename structure.
        OPENFILENAMEW ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);

        // set this to browser hwnd.
        ofn.hwndOwner = (HWND) m_plugin->getWindow();

        // Set lpstrFile[0] to '\0' so that GetOpenFileNameW does not 
        // use the contents of szFile to initialize itself.
        wchar_t szFile[32768];
        ofn.lpstrFile = szFile;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);

        wstring filter(filterName);
        filter.append(1, '\0');
        filter.append(filterPattern);
        filter.append(2, '\0');
        ofn.lpstrFilter = (wchar_t*) filter.c_str();
        ofn.nFilterIndex = 1;

        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrDefExt = NULL;
        
        std::string sUrl;
        m_plugin->getCurrentURL( sUrl );
        wstring wsTitle = getBrowseTitle( sUrl, m_locale );
        ofn.lpstrTitle = wsTitle.c_str();

        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
        ofn.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_ENABLESIZING;
        if (m_desc.majorVersion() > 1) {
            ofn.Flags |= OFN_NODEREFERENCELINKS;
        }

        // Our hook allows folder selection.  vPaths
        // populated by MyWinProc();
        ofn.Flags |= OFN_ENABLEHOOK;
        ofn.lpfnHook = MyHook;
        Context context = { NULL, &vPaths, locale() };
        ofn.lCustData = (LPARAM) &context;

        // Display the open dialog box, which populates context.m_paths
        // Note we don't have a good way to discriminate user cancel from
        // dialog error, so we treat them both the same for now.
        if (!GetOpenFileNameW(&ofn)) {
            failureCB(callbackArgument, tid, "FileBrowse.userCanceled",
                      "user canceled browse");
            return;
        }

        // ofn.lpstrFile only seems to be set for a double-clicking
        // a single item (in spite of what msdn claims).  In the case
        // of double-clicking broken shortcuts which were "fixed", 
        // it contains the fixed path, so use it.
        bpf::Path selPath(ofn.lpstrFile);
        if (!selPath.empty() && vPaths.size() == 1) {
            vPaths[0] = selPath;
        }
    }

    bp::Object* pObj = NULL;
    if (m_desc.majorVersion() == 1) {
        // version 1 applies filtering, recurses, etc
        unsigned int flags = 0;
        if (recurse) flags |= bp::pluginutil::kRecurse;
        if (includeGestureInfo) flags |= bp::pluginutil::kIncludeGestureInfo;
        pObj = bp::pluginutil::applyFilters(vPaths, mimetypes, flags, limit);
    } else {
        // version 2 and above just return what was selected
        bp::Map* m = new bp::Map;
        bp::List* l = new bp::List;
        vector<bpf::Path>::const_iterator it;
        for (it = vPaths.begin(); it != vPaths.end(); ++it) {
            bpf::Path path(*it);
            l->append(new bp::Path(path));
        }
        m->add("files", l);
        pObj = m;
    }
    successCB(callbackArgument, tid, pObj);
    delete pObj;
}

