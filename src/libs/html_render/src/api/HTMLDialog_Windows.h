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
 *  HtmlDialog_Windows.h
 *
 *  Declares bp::html::HtmlDialog, which is an ATL dialog that embeds a
 *  webbrowser control.  Basic common functionality is provided by this
 *  class - customization is achieved by specifying an EventListener.
 *
 *  Webbrowser events are hooked and passed to the event listener as
 *  appropriate.
 *
 *  Created by David Grigsby on 4/15/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef __HTMLDIALOG_H__
#define __HTMLDIALOG_H__

#include "BPUtils/bpurl.h"
#include "HtmlDialogRes_Windows.h"

#include <atlbase.h>
#include <atlcom.h>
#include <exdisp.h>
#include <exdispid.h>
#include <string>
#pragma warning( push )
#pragma warning( disable:4996 )
#include <wtl/atlapp.h>
#pragma warning( pop )
#include <wtl/atlcrack.h>
#include <wtl/atlmisc.h>


namespace bp {
namespace html {


class HtmlDialog :
    public CAxDialogImpl<HtmlDialog>,
    public CMessageFilter,
    public IDispEventImpl<IDC_IE, HtmlDialog>
{
// Event listener interface    
public:
    struct EventListener
    {
        // src:     source
        // sUrl:    proposed url, client may change
        // return:  true to allow, false to cancel
        virtual bool onBeforeNavigate( HtmlDialog& /*src*/,
                                       std::string& /*sUrl*/ )
        {
            return true;
        }
        
        virtual void onDocumentComplete( HtmlDialog& /*src*/ ) {}
        
        virtual void onNavigateComplete( HtmlDialog& /*src*/ ) {}
    };

    
// Construction/destruction
public:    
    HtmlDialog( const std::string& sTitle,
                const std::string& sPathToHtml,
                HICON hIcon = 0,
                EventListener* pListener = 0 );
    ~HtmlDialog();


// Public Methods
public:
    CComPtr<IWebBrowser2>   getBrowser();
    std::string             getUrl();

    bool                    dpiAware();
    float                   dpiScale();
    
    // Show the dialog.
    // nInitialZoomPcnt - 0 means leave zoom unchanged
    void                    show( int nWidth, int nHeight,
                                  int nInitialZoomPcnt = 0 );

   
// Our dialog id - needs to map to a dialog in a .rc file
// (and has to be public).
public:
    enum { IDD = IDD_HTMLDIALOG };

    
// Maps
private:    
    BEGIN_MSG_MAP(HtmlDialog)
        CHAIN_MSG_MAP(CAxDialogImpl<HtmlDialog>)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_DESTROY(OnDestroy)
    END_MSG_MAP()

    BEGIN_SINK_MAP(HtmlDialog)
        SINK_ENTRY(IDC_IE, DISPID_BEFORENAVIGATE2, OnBeforeNavigate2)
        SINK_ENTRY(IDC_IE, DISPID_DOCUMENTCOMPLETE, OnDocumentComplete)
        SINK_ENTRY(IDC_IE, DISPID_NAVIGATECOMPLETE2, OnNavigateComplete2)
    END_SINK_MAP()

    
// Windows Message handlers
private:    
    BOOL PreTranslateMessage(MSG* pMsg);
    BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam);
    void OnClose();
    void OnDestroy();

    
// Web browser event handlers
private:    
    void __stdcall OnBeforeNavigate2( IDispatch* pDisp,
                                      VARIANT* URL,
                                      VARIANT* Flags,
                                      VARIANT* TargetFrameName,
                                      VARIANT* PostData,
                                      VARIANT* Headers,
                                      VARIANT_BOOL* Cancel );
    void __stdcall OnDocumentComplete( LPDISPATCH pDisp, VARIANT* URL );
    void __stdcall OnNavigateComplete2( IDispatch* pDisp, VARIANT* URL );

    
// Internal Methods    
    bool                    getZoomPercent( int& nZoomPercent );
    bool                    setZoomPercent( int nZoomPercent );
    
// Internal State
private:
    std::string             m_sTitle;
    std::string             m_sPathToHtml;
    HICON                   m_hIcon;
    EventListener*          m_pListener;
    CAxWindow               m_awBrowser;
    CComPtr<IWebBrowser2>   m_spBrowser;
    int                     m_nZoomRestorePcnt; // 0 means no restore necessary
};


} // namespace html
} // namespace bp

#endif
