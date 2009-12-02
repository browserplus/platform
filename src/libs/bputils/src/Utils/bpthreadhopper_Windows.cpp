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

#include "bpthreadhopper.h"
#include "bpsync.h"
#include "api/bpuuid.h"
#include "api/bpstrutil.h"

#include <windows.h>

#define BP_PROXY_EVENT (WM_USER + 31)

struct DozeThreadHopperData {
    HWND hwnd;
    std::string className;

    long long thread_id;
    std::vector<std::pair<bp::thread::Hopper::InvokeFuncPtr, void *> > work;
    bp::sync::Mutex mtx;
    bool release;
};

static LRESULT CALLBACK
invokeClientCallbacks(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch (msg) {
        case BP_PROXY_EVENT: {
            DozeThreadHopperData * thd = (DozeThreadHopperData *) wParam;
            std::vector<std::pair<bp::thread::Hopper::InvokeFuncPtr, void *> >
                work;

            thd->mtx.lock();
            work = thd->work;
            thd->work.clear();
            thd->mtx.unlock();

            // guarantee we don't use this anymore
            thd = NULL;
            
            // now outside of our lock, relying only on local data,
            // invoke all of our client callbacks.  This allows one of
            // these invoked callbacks to delete us without ill
            // effect.
            while (work.size() > 0) {
                // peel work and context of the queue
                bp::thread::Hopper::InvokeFuncPtr invokeFunc =
                    work.front().first;
                void * context = work.front().second;
                work.erase(work.begin());
                invokeFunc(context);
            }
            
            return 0;
        }
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

bool
bp::thread::Hopper::initializeOnCurrentThread()
{
    // don't double init, please
    if (m_osSpecific != NULL) return false;

    DozeThreadHopperData * thd = new DozeThreadHopperData;

    // attain the current thread id
    thd->thread_id = ::GetCurrentThreadId();

    // allocate a temporal window class name
    thd->className = "BP Thread Hopper ephemeral win class ";
    {
        std::string uuid;
        if (bp::uuid::generate(uuid)) thd->className.append(uuid);
    }

    std::wstring wclassName = bp::strutil::utf8ToWide(thd->className);

    // allocate a window on this thread that we can post messages to
    WNDCLASS wc = { 0,                     // style
                    invokeClientCallbacks, // callback function
                    0,                     // extra bytes to alloc
                    0,                     // more extra bytes
                    NULL,                  // NULL hinstance!
                    NULL,                  // default icon, please
                    NULL,                  // no cursor, thanks
                    NULL,                  // keep yer background
                    NULL,                  // no menu name, thanks.
                    wclassName.c_str() };  // set the window class name

    ::RegisterClass(&wc);
    
    thd->hwnd = CreateWindowW(wclassName.c_str(),
                              L"BP ThreadHopper Window",
                              0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
    
    thd->release = false;

    m_osSpecific = (void *) thd;

    return true;
}

bool
bp::thread::Hopper::invokeOnThread(InvokeFuncPtr invokeFunc, void * context)
{
    DozeThreadHopperData * thd = (DozeThreadHopperData *) m_osSpecific;

    thd->mtx.lock();

    thd->work.push_back(std::pair<InvokeFuncPtr, void *>(invokeFunc, context));
    PostMessage(thd->hwnd, BP_PROXY_EVENT, (WPARAM) m_osSpecific,
                      NULL);
    thd->mtx.unlock();

    return true;
}

bp::thread::Hopper::~Hopper()
{
    DozeThreadHopperData * thd = (DozeThreadHopperData *) m_osSpecific;

	thd->mtx.lock();

    ::DestroyWindow(thd->hwnd);

    std::wstring wclassName = bp::strutil::utf8ToWide(thd->className);
	::UnregisterClass(wclassName.c_str(), NULL);

    thd->mtx.unlock();

    delete thd;
    m_osSpecific = NULL;
}

void
bp::thread::Hopper::processOutstandingRequests()
{
    DozeThreadHopperData * thd = (DozeThreadHopperData *) m_osSpecific;

    thd->mtx.lock();

    while (thd->work.size() > 0) {
        bp::thread::Hopper::InvokeFuncPtr invokeFunc = thd->work.front().first;
        void * context = thd->work.front().second;
        thd->work.erase(thd->work.begin());

        thd->mtx.unlock();
        // call client callback outside of lock.
        invokeFunc(context);
        thd->mtx.lock();
    }

    thd->mtx.unlock();
}
