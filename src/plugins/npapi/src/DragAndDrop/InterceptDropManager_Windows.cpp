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
 *  DropManager_Windows.cpp
 *  BrowserPlusPlugin
 *
 *  Created by Gordon Durand on 5/29/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include "NPAPIPlugin.h"
#include "InterceptDropManager.h"
#include "nputils.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bptime.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bperrorutil.h"

#include <npapi/npapi.h>
#include <npapi/npruntime.h>
#include <npapi/npupp.h>

#include <windows.h>
#include <ShellAPI.h>
#include <lm.h>
#include <objbase.h>
#include <oleidl.h>
#include <shlobj.h>

using namespace std;

// no "unused args" or "conditional expression is a constant" warnings
// (second warning from NPVariant macros)
#pragma warning (disable : 4100 4127)

class WindowsDropManager : public virtual InterceptDropManager,
                           public virtual IDropTarget
{
public:
    WindowsDropManager(NPP instance,
                       NPWindow* window,
                       IDropListener* dl);
    ~WindowsDropManager();
    
private:
    //IUnknown methods
    virtual HRESULT __stdcall QueryInterface(REFIID riid,
                                             void** ppvObject);
    ULONG __stdcall AddRef(void);
    ULONG __stdcall Release(void);

    // DropManager overrides

    virtual bool addTarget(const std::string& name,
                           const std::set<std::string>& mimeTypes,
                           bool includeGestureInfo,
                           unsigned int limit);
    virtual bool addTarget(const std::string& name,
                           const std::string& version);
    virtual bool removeTarget(const std::string& name);

    //IDropTarget methods

    virtual HRESULT __stdcall DragEnter(IDataObject* pDataObj,
                                        DWORD grfKeyState,
                                        POINTL pt,
                                        DWORD* pdwEffect);

    virtual HRESULT __stdcall DragOver(DWORD grfKeyState,
                                       POINTL pt,
                                       DWORD* pdwEffect);

    virtual HRESULT __stdcall DragLeave(void);

    virtual HRESULT __stdcall Drop(IDataObject* pDataObj,
                                   DWORD grfKeyState,
                                   POINTL pt,
                                   DWORD* pdwEffect); 

protected:
    virtual void derivedSetWindow(NPWindow* window);

private:

    bool createOverlay(const std::string& name);

    std::vector<boost::filesystem::path> getDragItems(IDataObject* pDataObj);

    // take global coordinates and convert them to window relative coords.
    void localizeCoords(LONG &x, LONG &y);

    void sendHover(bool onOff);
    bool isValidDataSource(IDataObject* pDataObj) {
        // TODO: what is the purpose of this function?
        return true;
    }

    IDataObject* m_pDataObject;
    LONG m_refCount;
    HWND m_hWnd;
    HWND m_pluginHWnd;

    // older firefox browsers require extra hacks to re-arrange the
    // stacking order: each drop target has a transparent overlay which we 
    // force to the top of the z-order.  events are forwarded to parent window.
    // this technique is not used on other browsers.
    bool m_isOldFirefoxBrowser;
    std::string m_className;
    ATOM m_atom;
    HWND m_dndHWnd;
    static LRESULT CALLBACK dropWindowCallback(HWND hwnd, 
                                               UINT msg,
                                               WPARAM wParam,
                                               LPARAM lParam);
};


IDropManager*
InterceptDropManager::allocate(NPP instance,
                               NPWindow* window,
                               IDropListener* dl)
{
    return new WindowsDropManager(instance, window, dl);
}


void
WindowsDropManager::derivedSetWindow(NPWindow* window)
{
    BPLOG_INFO_STRM("SetWindow: " << window << " " << 
                     window->x << "," << window->y);
}


WindowsDropManager::WindowsDropManager(NPP instance,
                                       NPWindow* window,
                                       IDropListener* dl) 
    : InterceptDropManager(instance, window, dl), 
      m_pDataObject(NULL), m_refCount(0), m_atom(0)
{
    try {
        bp::SemanticVersion baseVersion;
        (void) baseVersion.parse("3.6.0");
        NPAPIPlugin* plugin = (NPAPIPlugin*)instance->pdata;
        m_isOldFirefoxBrowser = plugin->getBrowserInfo().version().compare(baseVersion) < 0;
    } catch (const bp::error::Exception& e) {
        BPLOG_ERROR_STRM("caught " << e.what());
        m_isOldFirefoxBrowser = false;
    }
    m_pluginHWnd = (HWND)(window->window);
    m_hWnd = GetParent(m_pluginHWnd);
    OleInitialize(0);
    BPLOG_INFO_STRM("allocated new windows drop manager: " <<  m_hWnd);
	BPLOG_INFO_STRM("intercept behavior: " <<  
		(m_isOldFirefoxBrowser ? "ffx < 3.6" : "new or non-firefox"));
}


WindowsDropManager::~WindowsDropManager()
{
    if (!m_targets.empty()) {
        if (m_isOldFirefoxBrowser) {
            if (m_dndHWnd) {
                ::RevokeDragDrop(m_dndHWnd);
                ::DestroyWindow(m_dndHWnd);
                m_dndHWnd = NULL;
            }
        } else {
            ::RevokeDragDrop(m_hWnd);
        }
    } 
    if (!m_className.empty()) {
        BPLOG_INFO_STRM("unregister " << m_className);
        ::UnregisterClassW(bp::strutil::utf8ToWide(m_className).c_str(), NULL);
    }
    OleUninitialize();
    BPLOG_INFO_STRM("freed windows drop manager: " << this);
}


bool
WindowsDropManager::createOverlay(const std::string& name)
{
    HWND hwnd = NULL;
    try {
        if (m_targets.size() == 1) {
            if (m_isOldFirefoxBrowser) {
                if (m_atom == 0) {
                    // window class name must be scoped to this instance of 
                    // dropmanager.  trying to share a classname results in
                    // lotsa crashes in the presence of multiple dropmanagers
                    // to avoid name collisions during reloads, form name from
                    // instance address and timestamp
                    std::stringstream sstr;
                    BPTime now;
                    sstr << "BrowserPlusDropTargetWindowClass-" 
                         << this << "-" << now.asString();
                    m_className = sstr.str();
                    BPLOG_INFO_STRM("Try to register WindowsDropManager class " 
                                    << m_className);
                    wstring wclassName = bp::strutil::utf8ToWide(m_className);
                    WNDCLASSEX wndClass = {
                        sizeof(WNDCLASSEX),
                        0,                                          // no style
                        WindowsDropManager::dropWindowCallback,     // wndProc
                        0,                                          // no extra bytes
                        0,                                          // no extra bytes
                        NULL,                                       // no instance
                        NULL,                                       // no icon
                        NULL,                                       // no cursor
                        NULL,                                       // no background brush
                        NULL,                                       // no menu
                        wclassName.c_str(),                         // class name
                        NULL                                        // no small icon
                    };
                    m_atom = ::RegisterClassExW(&wndClass);
                    if (m_atom == 0) {
                        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
                            throw std::string("::RegisterClassEx failed");
                        } else {
                            // if this happens, havoc can ensue with multiple
                            // frames and reloads.  it may work, it may not
                            BPLOG_WARN("WindowsDropManager class already existed");
                        }
                    }
                }
                    
                // create transparent overlay
                RECT r;
                if (!::GetClientRect(m_hWnd, &r)) {
                    throw std::string("::GetClientRect failed");
                }
                wstring wclassName = bp::strutil::utf8ToWide(m_className);
                m_dndHWnd = CreateWindowExW(WS_EX_ACCEPTFILES 
                                            | WS_EX_TRANSPARENT
                                            | WS_EX_NOPARENTNOTIFY,
                                            wclassName.c_str(), 
                                            L"",
                                            WS_CHILD, 
                                            0, 0,
                                            r.right - r.left,
                                            r.bottom - r.top,
                                            m_hWnd,
                                            NULL,
                                            NULL,
                                            NULL);
                if (m_dndHWnd == NULL) {
                    throw std::string("CreateWindowEx failed");
                }
                ::SetWindowPos(m_dndHWnd, HWND_TOP, 0, 0, 0, 0,
                               SWP_NOREDRAW | SWP_NOSENDCHANGING
                               | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
                HRESULT res = ::RegisterDragDrop(m_dndHWnd, this);
                if (res != S_OK) {
                    throw std::string("RegisterDragDrop failed");
                }
                BPLOG_INFO_STRM("added drop target window: " <<
                                BP_HEX_MANIP << long(m_dndHWnd));
            } else {
                ::RevokeDragDrop(m_hWnd);
                HRESULT res = ::RegisterDragDrop(m_hWnd, this);
                if (res != S_OK) {
                    throw std::string("::RegisterDragDrop failed");
                }
            }
        }
    } catch (const std::string& msg) {
        std::string errMsg = bp::error::lastErrorString();
        BPLOG_INFO_STRM("WindowsDropManager::createOverlay" 
                        << "(" << name << ")" << ":  " << msg
                        << ": " << errMsg);
        if (hwnd) ::DestroyWindow(hwnd);
        InterceptDropManager::removeTarget(name);
        return false;
    }
    return true;
}


bool
WindowsDropManager::addTarget(const std::string& name,
                              const std::set<std::string>& mimeTypes,
                              bool includeGestureInfo,
                              unsigned int limit)
{
    BPLOG_INFO_STRM("addTarget(" << name << "), " 
                    << m_targets.size() << " existing targets");
    bool rval = InterceptDropManager::addTarget(name, mimeTypes,
                                                includeGestureInfo, limit);
    if (rval) {
        rval = createOverlay(name);
    } else {
        BPLOG_INFO_STRM("DropManager::addTarget() failed");
    }
    return rval;
}


bool
WindowsDropManager::addTarget(const std::string& name,
                              const std::string& version)
{
    BPLOG_INFO_STRM("addTarget(" << name << "), " 
                    << m_targets.size() << " existing targets");
    bool rval = InterceptDropManager::addTarget(name, version);
    if (rval) {
        rval = createOverlay(name);
    } else {
        BPLOG_INFO_STRM("DropManager::addTarget() failed");
    }
    return rval;
}


bool
WindowsDropManager::removeTarget(const std::string& name)
{
    BPLOG_INFO_STRM("removeTarget(" << name << ")");
    bool rval = InterceptDropManager::removeTarget(name);
    if (rval) {
        if (m_targets.empty()) {
            if (m_isOldFirefoxBrowser) {
                ::RevokeDragDrop(m_dndHWnd);
                ::DestroyWindow(m_dndHWnd);
                m_dndHWnd = NULL;
            } else {
                ::RevokeDragDrop(m_hWnd);
            }
        }
    }
    return rval;
}


void
WindowsDropManager::localizeCoords(LONG& x,
                                   LONG& y)
{
    // convert to local coordinates, using the
    // plugin's window which is fixed at the upper left corner of the
    // page.  This accounts for scrolling automagically.
    RECT r;
    if (!GetWindowRect(m_pluginHWnd, &r)) return;
    x -= r.left;
    y -= r.top;
    
    return;
}


HRESULT __stdcall
WindowsDropManager::QueryInterface(REFIID iid, 
                                   void** ppvObject)
{
    if (iid == IID_IDropTarget || iid == IID_IUnknown) {
        AddRef();
        *ppvObject = this;
        return S_OK;
    } 
    *ppvObject = NULL;
    return E_NOINTERFACE;
}


ULONG __stdcall
WindowsDropManager::AddRef(void)
{
	// ignore
    return 1;
}


ULONG __stdcall
WindowsDropManager::Release(void)
{
	// we don't want to be double freed here.
    return 1;
}


HRESULT __stdcall 
WindowsDropManager::DragEnter(IDataObject* pDataObj,
                              DWORD grfKeyState,
                              POINTL pt,
                              DWORD* pdwEffect)
{
    BPLOG_INFO_STRM("DragEnter: " << pt.x << "," << pt.y);
    std::vector<boost::filesystem::path> dragItems = getDragItems(pDataObj);
    handleMouseEnter(dragItems);
    return S_OK;
}


HRESULT __stdcall 
WindowsDropManager::DragOver(DWORD grfKeyState,
                             POINTL pt,
                             DWORD* pdwEffect)
{
    BPLOG_INFO_STRM("DragOver: " << pt.x << "," << pt.y);
	localizeCoords(pt.x, pt.y);
    if (handleMouseDrag((short int) pt.x, (short int) pt.y)) {
        *pdwEffect = DROPEFFECT_COPY;
    } else {
        *pdwEffect = DROPEFFECT_NONE;
    }
    return S_OK;
}


HRESULT __stdcall 
WindowsDropManager::DragLeave(void)
{
    BPLOG_INFO("DragLeave");
    handleMouseExit(true);
    return S_OK;
}


HRESULT __stdcall 
WindowsDropManager::Drop(IDataObject* pDataObject,
                         DWORD grfKeyState,
                         POINTL pt,
                         DWORD* pdwEffect)
{
    BPLOG_INFO_STRM("Drop " << pt.x << "," << pt.y);
    localizeCoords(pt.x, pt.y);
    if (handleMouseDrop((short int) pt.x, (short int) pt.y)) {
        *pdwEffect = DROPEFFECT_COPY;
    } else {
        *pdwEffect = DROPEFFECT_NONE;
    }
    handleMouseExit(true);
    return S_OK;
}


LRESULT CALLBACK
WindowsDropManager::dropWindowCallback(HWND hwnd, 
                                       UINT msg,
                                       WPARAM wParam, 
                                       LPARAM lParam)
{ 
    LRESULT rval = 0L;
    switch (msg) {
    case WM_CREATE: {
        (void) ::SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
                              SWP_NOREDRAW | SWP_NOSENDCHANGING
                              | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        rval = 1L;
        break;
    }
    case WM_WINDOWPOSCHANGING: {
        if (::GetTopWindow(::GetParent(hwnd)) != hwnd) {
            // we're on top, dammit!
            (void) ::SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
                                  SWP_NOREDRAW | SWP_NOSENDCHANGING
                                  | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        }
        rval = 1L;
        break;
    }
    case WM_MOVE:
    case WM_SIZE: {
        RECT r = {0, 0, 0, 0};
        ::GetClientRect(::GetParent(hwnd), &r);
        ::MoveWindow(hwnd, 0, 0, r.right - r.left,
                     r.bottom - r.top, false);
        break;
    }
    default:
        rval = ::DefWindowProc(hwnd, msg, wParam, lParam);
    }

    // bugs 1631592 and 2726247 
	// certain messages we must forward to the parent window (browser),
	// others we should not.  For instance if we forward WM_MOUSEWHEEL
	// we can get event ringing (supposedly) and application spinning.
	// We take a conservative approach and only forward messages 
	// related to mouse events.  There is a possibility that other messages
	// will need to be forwarded in the future.
    if (rval == 0 && (0x200 <= msg && msg <= 0x209)) {
        HWND next = ::GetParent(hwnd);
        if (next != NULL) {
           ::PostMessage(next, msg, wParam, lParam);
        }
	} 
	return rval;
}


std::vector<boost::filesystem::path>
WindowsDropManager::getDragItems(IDataObject* pDataObject)
{
    std::vector<boost::filesystem::path> results;

    // construct a FORMATETC object
    FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stgmed;

    // See if the dataobject contains any TEXT stored as a HGLOBAL
    if (pDataObject->QueryGetData(&fmtetc) == S_OK) {
        // Yippie! the data is there, so go get it!
        if (pDataObject->GetData(&fmtetc, &stgmed) == S_OK) {
            // we asked for the data as a HGLOBAL, so access it appropriately
            HDROP drop = (HDROP)GlobalLock(stgmed.hGlobal);
            int num = DragQueryFileW(drop, 0xFFFFFFFF, NULL, 0);
            for (int i = 0; i < num; i++) {
                int nameLen = DragQueryFileW(drop, i, NULL, 0) + 1;
                wchar_t* fileName = new wchar_t[nameLen];
                DragQueryFileW(drop, i, fileName, nameLen);
                std::wstring wName(fileName);
                boost::filesystem::path path(wName);
                results.push_back(path);
                delete[] fileName;
            }

            GlobalUnlock(stgmed.hGlobal);

            // release the data using the COM API
            ReleaseStgMedium(&stgmed);
        }
    }
    
    return results;
}
