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

#include "api/bprunloop.h"

#include <stdlib.h>
#include <iostream>
#include <windows.h>
#include "api/bpstrutil.h"
#include "api/bperrorutil.h"
#include "api/bpuuid.h"

using namespace std;


#define BPWINRUNLOOP_EVENT (WM_USER + 33)
#define BPWINQUIT_EVENT (WM_USER + 34)

struct Win32RunLoopData 
{
    Win32RunLoopData() : window(NULL), classInstance(NULL), className(),
                         m_workQueue(NULL), lock(NULL),
                         onEventFunc(NULL), onEventCookie(NULL) {
    }
    // win32 gunk
    HWND window;
    HINSTANCE classInstance;
    std::string className;
    
    // queue of events
    std::vector<bp::runloop::Event> * m_workQueue;
    // pointer to lock used by runloop object
    bp::sync::Mutex * lock;
    // who to deliver the event t
    bp::runloop::eventCallBack onEventFunc;
    void * onEventCookie;
};

static LRESULT CALLBACK
win32GotEventCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case BPWINRUNLOOP_EVENT:
        {
            Win32RunLoopData * wrld = (Win32RunLoopData *) wParam;
            wrld->lock->lock();
            while (wrld->m_workQueue->size() > 0) {
                bp::runloop::Event work(*(wrld->m_workQueue->begin()));
                wrld->m_workQueue->erase(wrld->m_workQueue->begin());
                if (wrld->onEventFunc) {
                    wrld->lock->unlock();
                    wrld->onEventFunc(wrld->onEventCookie, work);
                    wrld->lock->lock();
                }
            }
            wrld->lock->unlock();
            return 0;
        }
    }
    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

static HWND createWindow(std::string className, HINSTANCE & classInstance) 
{    
    wstring wclassName = bp::strutil::utf8ToWide(className);

    WNDCLASS wc;
    if (::GetClassInfoW(classInstance, wclassName.c_str(), &wc) == 0)
    {
        // Initialize.
        ::ZeroMemory(&wc, sizeof(wc));
        // Main window class structure.
        wc.style = /*CS_VREDRAW | CS_HREDRAW |*/ CS_GLOBALCLASS;
        wc.lpfnWndProc = static_cast<WNDPROC>(win32GotEventCallback);
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.lpszClassName = wclassName.c_str();
        wc.hInstance = classInstance;
        ::RegisterClass(&wc);
    }

    HWND window = NULL;
    window = ::CreateWindowW(wc.lpszClassName,
                             L"BrowserPlus RunLoop window",
                             0L, 0, 0, 0, 0,
                             NULL, NULL, classInstance, NULL);

    return window;
}

static void destroyWindow(HWND window,
                          std::string className,
                          HINSTANCE classInstance)
{
    if (window != NULL)
    {
        wstring wclassName = bp::strutil::utf8ToWide(className);
        ::DestroyWindow(window);
        ::UnregisterClassW(wclassName.c_str(), classInstance);
    }
}

void
bp::runloop::RunLoop::init()
{
    Win32RunLoopData * wrld = new Win32RunLoopData;

    wrld->className = "BrowserPlus hidden window class ";
    {
        std::string uuid;
        if (bp::uuid::generate(uuid)) wrld->className.append(uuid);
    }
    wrld->window = createWindow(wrld->className, wrld->classInstance);

    wrld->onEventFunc = m_onEvent;
    wrld->onEventCookie = m_onEventCookie;
    wrld->m_workQueue = &m_workQueue;

    wrld->lock = &m_lock;

    m_osSpecific = (void *) wrld;
}

void
bp::runloop::RunLoop::shutdown()
{
    BPASSERT(m_osSpecific != NULL);
    Win32RunLoopData * wrld = (Win32RunLoopData *) m_osSpecific;

	destroyWindow(wrld->window, wrld->className, wrld->classInstance);
    
    delete wrld;
    m_osSpecific = NULL;
}

void
bp::runloop::RunLoop::run()
{
    MSG msg;

    int rv;

    // dg: Get messages for all windows owned by this thread.
    while ((rv = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (msg.message == BPWINQUIT_EVENT)
        {
            break;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // process all pending events before shutting down
    win32GotEventCallback(NULL, BPWINRUNLOOP_EVENT, (WPARAM) m_osSpecific, 0);
}

bool
bp::runloop::RunLoop::sendEvent(Event e)
{
    m_lock.lock();
    BPASSERT(m_osSpecific != NULL);

    Win32RunLoopData * wrld = (Win32RunLoopData *) m_osSpecific;

    m_workQueue.push_back(e);

    ::PostMessage(wrld->window,
                  BPWINRUNLOOP_EVENT,
                  reinterpret_cast<WPARAM>(wrld),
                  0);
    
    m_lock.unlock();

    return true;
}

void
bp::runloop::RunLoop::stop()
{
    Win32RunLoopData * wrld = (Win32RunLoopData *) m_osSpecific;
    BPASSERT(wrld != NULL && wrld->window != NULL);

    ::PostMessage(wrld->window, BPWINQUIT_EVENT, 0, 0);
}

void
bp::runloop::RunLoop::setCallBacks(eventCallBack onEvent, void * onEventCookie)
{
    Win32RunLoopData * wrld = (Win32RunLoopData *) m_osSpecific;

    m_onEvent = onEvent;
    m_onEventCookie = onEventCookie;
    if (wrld) {
        wrld->onEventFunc = onEvent;
        wrld->onEventCookie = onEventCookie;
    }
}
