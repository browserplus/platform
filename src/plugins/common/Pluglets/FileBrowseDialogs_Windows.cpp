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
 *  FileBrowseDialogs_Windows.cpp
 */

#define _ATL_APARTMENT_THREADED
#include "FileBrowseDialogs_Windows.h"

#include <atlbase.h>
#include <atlcom.h>
#include <CommDlg.h>
#include <Dlgs.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <vector>

#include "BPUtils/bpfile.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpstrutil.h"
#include "FileBrowsePluglet.h"
#include "platform_utils/bplocalization.h"


using namespace std;
namespace bpf = bp::file;
namespace bpl = bp::localization;


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


class MyEventHandler :
    public CComObjectRootEx<CComSingleThreadModel>,
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


bool runFileOpenDialog(const FileOpenDialogParms& parms,
                       vector<bpf::Path>& vPaths)
{
    CComPtr<IFileOpenDialog> pDlg;
    COMDLG_FILTERSPEC filters[] = {
        parms.filterName.c_str(),
        parms.filterPattern.c_str()
    };
    HRESULT hr = pDlg.CoCreateInstance(__uuidof(FileOpenDialog));
    if (FAILED(hr)) {
        BPLOG_ERROR("unable to create FileOpenDialog");
        return false;
    }

    pDlg->SetTitle(parms.title.c_str());
    pDlg->SetFileTypes(_countof(filters), filters);

    DWORD flags = 0;
    pDlg->GetOptions(&flags);
    flags |= FOS_ALLOWMULTISELECT | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST
             | FOS_FILEMUSTEXIST | FOS_DONTADDTORECENT;
    if (!parms.chaseLinks) {
        flags |= FOS_NODEREFERENCELINKS;
    }
    pDlg->SetOptions(flags);

    // add custom controls
    CComQIPtr<IFileDialogCustomize> pfdc = pDlg;
    if (!pfdc) {
        BPLOG_ERROR("unable to get IFileDialogCustomize ptr "
                    "(should not happen!)");;
        return false;
    }

    // add select button.
    string lstr;
    (void) bpl::getLocalizedString(FileBrowsePluglet::kSelectKey,
                                   parms.locale, lstr);
    hr = pfdc->AddPushButton(MyEventHandler::kSelectButtonId,
                             bp::strutil::utf8ToWide(lstr).c_str());
    if (FAILED(hr)) {
        BPLOG_ERROR("unable to add Select button to dialog");
        return false;
    }

    // add event listener
    CComObjectStackEx<MyEventHandler> evh;
    evh.setPaths(&vPaths);
    CComQIPtr<IFileDialogEvents> pEvents = evh.GetUnknown();
    DWORD cookie;
    hr = pDlg->Advise(pEvents, &cookie);
    bool advised = SUCCEEDED(hr);

    // show dialog.  vPaths populated by our event listener
    hr = pDlg->Show(parms.parentWnd);
    if (advised) {
        pDlg->Unadvise(cookie);
    }

    return true;
}



bool runFileSaveDialog(const FileSaveDialogParms& parms,
                       bp::file::Path& path)
{
    // Setup the openfilename structure.
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);

    ofn.hwndOwner = (HWND) parms.parentWnd;
    // hInstance - ignored
    ofn.lpstrFilter = NULL;
    ofn.lpstrCustomFilter = NULL;
    // nMaxCustFilter - ignored
    ofn.nFilterIndex = 0;

    wchar_t szFile[32768];
    ofn.lpstrFile = szFile;
    wcscpy_s(szFile, parms.initialName.c_str());
    
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);

    ofn.lpstrFileTitle = NULL;
    // nMaxFileTitle - ignored
    
    // TODO: review
    ofn.lpstrInitialDir = NULL;
    
    ofn.lpstrTitle = parms.title.c_str();
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    // nFileOffset - output
    // nFileExtension - output
    ofn.lpstrDefExt = NULL;
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;


    // Run the dialog.
    BOOL bStat = GetSaveFileNameW(&ofn);
    if (!bStat && CommDlgExtendedError()) {
        BPLOG_ERROR("GetSaveFileNameW failed.");;
        return false;
    }

    path = ofn.lpstrFile;
    
    return true;
}

