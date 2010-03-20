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
 *  HtmlDialog_Windows.cpp
 *
 *  Created by David Grigsby on 4/15/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#include "HTMLDialog_Windows.h"

#include "BPUtils/BPLog.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bpfile.h"
#include "ComUtils_Windows.h"
#include "RegUtils_Windows.h"

#include <comutil.h>

namespace bp {
namespace html {


//////////////////////////////////////////////////////////////////////
// HtmlDialog
//

HtmlDialog::HtmlDialog( const std::string& sTitle,
                        const std::string& sPathToHtml,
                        HICON hIcon,
                        EventListener* pListener ) :
    m_sTitle( sTitle ),
    m_sPathToHtml( sPathToHtml ),
    m_hIcon( hIcon ),
    m_pListener( pListener ),
    m_awBrowser(),
    m_spBrowser(),
    m_nZoomRestorePcnt( 0 )
{
}


HtmlDialog::~HtmlDialog()
{
}


CComPtr<IWebBrowser2> HtmlDialog::getBrowser()
{
    // TODO: could check underlying pointer.
    return m_spBrowser;
}


std::string HtmlDialog::getUrl()
{
    return m_sPathToHtml;
}

void HtmlDialog::show( int nCliWidth, int nCliHeight,
                       int nInitialZoomPcnt )
{
    BPLOG_INFO_STRM( "Client Width/Height/Zoom: " <<
                     nCliWidth << "/" << nCliHeight << "/" <<
                     nInitialZoomPcnt );
            
    if (nInitialZoomPcnt != 0)
    {
        getZoomPercent( m_nZoomRestorePcnt );
        setZoomPercent( nInitialZoomPcnt );
    }
    
    // Figure out what window size will be necessary to get the
    // requested client window width and height.
    // TODO: compensate for zoom?
    int nWndWidth  = nCliWidth +
                     GetSystemMetrics( SM_CXFIXEDFRAME ) * 2;
    int nWndHeight = nCliHeight +
                     GetSystemMetrics( SM_CYFIXEDFRAME ) * 2 +
                     GetSystemMetrics( SM_CYCAPTION );
    BPLOG_INFO_STRM( "Wnd Width/Height: " << nWndWidth << "/" << nWndHeight );
    
    // Adjust dialog size.
    SetWindowPos( NULL, 0, 0, nWndWidth, nWndHeight, SWP_NOMOVE|SWP_NOZORDER );

    // Set browser window to occupy entire dialog.
    CRect rcClient;
    GetClientRect( &rcClient );
    m_awBrowser.SetWindowPos( NULL, rcClient, SWP_NOZORDER );

    // Make us visble.
    SetWindowPos( NULL, 0, 0, 0, 0,
                  SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER );
}


BOOL HtmlDialog::PreTranslateMessage( MSG* pMsg )
{
/*    
    // This was stolen from an SDI app using a form view.
    //
    // Pass keyboard messages along to the child window that has the focus.
    // When the browser has the focus, the message is passed to its containing
    // CAxWindow, which lets the control handle the message.

    if((pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST) &&
       (pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST))
        return FALSE;

    HWND hWndCtl = ::GetFocus();

    if(IsChild(hWndCtl))
    {
        // find a direct child of the dialog from the window that has focus
        while(::GetParent(hWndCtl) != m_hWnd)
            hWndCtl = ::GetParent(hWndCtl);

        // give control a chance to translate this message
        if(::SendMessage(hWndCtl, WM_FORWARDMSG, 0, (LPARAM)pMsg) != 0)
            return TRUE;
    }
*/
    // A normal control has the focus, so call IsDialogMessage() so that
    // the dialog shortcut keys work (TAB, etc.)
    return IsDialogMessage(pMsg);
}


BOOL HtmlDialog::OnInitDialog( HWND hwndFocus, LPARAM /*lParam*/ )
{
    SetWindowPos( NULL, 0, 0, 0, 0,
                  SWP_HIDEWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER );
//  ModifyStyle( WS_VISIBLE, 0 );

    CenterWindow();

    std::wstring wTitle( bp::strutil::utf8ToWide( m_sTitle ) );
    SetWindowTextW( wTitle.c_str() );

    // If caller gave us an icon, use it.
    if (m_hIcon)
    {
        SetIcon( m_hIcon, TRUE );
        SetIcon( m_hIcon, FALSE );
    }
    
    // From here down is setup for the embedded browser.
    //
    m_awBrowser.Attach( GetDlgItem( IDC_IE ) );

//  m_awBrowser.SetExternalUIHandler( this );

    CComPtr<IAxWinAmbientDispatch> spAmbient;
    m_awBrowser.QueryHost( &spAmbient);
    if (!spAmbient)
    {
        BPLOG_ERROR( "QueryHost() failed!" );
        return FALSE;
    }

    // Disable the context menu.
    spAmbient->put_AllowContextMenu( VARIANT_FALSE );

    // Customize the browser host.
    DWORD dwFlags = DOCHOSTUIFLAG_DIALOG |
                    DOCHOSTUIFLAG_NO3DBORDER |
                    DOCHOSTUIFLAG_SCROLL_NO |
                    DOCHOSTUIFLAG_OPENNEWWIN |
                    DOCHOSTUIFLAG_DPI_AWARE;
    spAmbient->put_DocHostFlags( dwFlags );

    // Modify styles as needed.
//  ModifyStyle( WS_CAPTION, 0 );

    // Setup our IWebBrowser2 ptr.
    m_awBrowser.QueryControl( &m_spBrowser );
    if (!m_spBrowser)
    {
        BPLOG_ERROR( "QueryControl() failed!" );
        return FALSE;
    }

    // Disable drag & drop.
    m_spBrowser->put_RegisterAsDropTarget( VARIANT_FALSE );
    
    // Navigate to the proper content.
    std::wstring wsNativePath = bp::file::nativeFromUtf8( m_sPathToHtml );
    _bstr_t bsPath( wsNativePath.c_str() );
    CComVariant vt;
    HRESULT hr = m_spBrowser->Navigate( bsPath, &vt, &vt, &vt, &vt );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "m_spBrowser->Navigate() failed!", hr );
        return FALSE;
    }

    return FALSE;
}


void HtmlDialog::OnClose()
{
    // TODO: could allow provision of return code.
    EndDialog( 0 );
}


void HtmlDialog::OnDestroy()
{
    // TODO: is there a better place for this?
    // For Windows IE there is a user-scoped persistent zoom value.
    // We need to restore it if we changed it.
    // Note: On IE8 anyway it would be preferable to use
    // OLECMDIDF_OPTICAL_ZOOM_NOPERSIST, but don't know how at the moment.
    if (m_nZoomRestorePcnt != 0) {
        setZoomPercent( m_nZoomRestorePcnt );
    }
    
    // Release our interface on the browser control.
    if (m_spBrowser) {
        m_spBrowser.Release();
    }

    SetMsgHandled(false);
}


void __stdcall HtmlDialog::OnBeforeNavigate2( IDispatch* /*pDisp*/,
                                              VARIANT* pvtUrl,
                                              VARIANT* /*Flags*/,
                                              VARIANT* /*TargetFrameName*/,
                                              VARIANT* /*PostData*/,
                                              VARIANT* /*Headers*/,
                                              VARIANT_BOOL* pvbCancel )
{
    if (m_pListener)
    {
        std::string sUrl = bp::strutil::wideToUtf8(
                               std::wstring( _bstr_t( pvtUrl->bstrVal ) ) );
        
        bool bRet = m_pListener->onBeforeNavigate( *this, sUrl );
        
        // TODO: update pvtUrl
        *pvbCancel = bRet ? VARIANT_FALSE : VARIANT_TRUE;
    }
}


void __stdcall HtmlDialog::OnDocumentComplete( LPDISPATCH /*pDisp*/,
                                               VARIANT* /*URL*/ )
{
    if (m_pListener)
    {
        m_pListener->onDocumentComplete( *this );
    }
}


void __stdcall HtmlDialog::OnNavigateComplete2( IDispatch* /*pDisp*/,
                                                VARIANT* /*URL*/ )
{
    if (m_pListener)
    {
        m_pListener->onNavigateComplete( *this );
    }
}


bool HtmlDialog::dpiAware()
{
    static bool dpiAware = false;
    static bool dpiCheckComplete = false;

    // only run check once per load and cache results, minimizing reg probing.
    if (!dpiCheckComplete) {
        // IE 8 is dpi aware
        std::string ieVersion =
            bp::registry::readString(
                "HKLM\\SOFTWARE\\Microsoft\\Internet Explorer", "Version" );

        dpiAware = ( ieVersion.length() > 0 &&
                     !(ieVersion.substr( 0, 1 ).compare( "8" )) );
        
        BPLOG_INFO_STRM( "Installed IE version: " << ieVersion  << "("
                         << (dpiAware ? "" : "NOT ") << "DPI aware)" );    

        dpiCheckComplete = true;
    }
    
    return dpiAware;
}


float HtmlDialog::dpiScale()
{
    static float gScale = 0.0;

    if (gScale == 0.0) {
        HDC hdc = ::GetDC(NULL);
        gScale = ::GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
        ::ReleaseDC(NULL, hdc);       
    }

    BPLOG_INFO_STRM( "dpiScale is: " << gScale );
    
    return gScale;
}


bool HtmlDialog::getZoomPercent( int& nZoomPercent )
{
    if (!m_spBrowser) {
        BPLOG_ERROR( "getZoomPercent called with null m_spBrowser." );
        return false;
    }
    
    CComVariant vtOut;
    HRESULT hr = m_spBrowser->ExecWB( OLECMDID_OPTICAL_ZOOM,
                                      OLECMDEXECOPT_DONTPROMPTUSER,
                                      NULL,
                                      &vtOut );
    if (FAILED( hr )) {
        BPLOG_COM_ERROR( "get optical_zoom failed.", hr );
        return false;
    }

    nZoomPercent = vtOut.lVal;
    BPLOG_INFO_STRM( "IE Zoom is: " << nZoomPercent << "%." );
    return true;
}


bool HtmlDialog::setZoomPercent( int nZoomPercent )
{
    if (!m_spBrowser) {
        BPLOG_ERROR( "setZoomPercent called with null m_spBrowser." );
        return false;
    }

    CComVariant vtIn( nZoomPercent );
    
    CComVariant vtOut;
    HRESULT hr = m_spBrowser->ExecWB( OLECMDID_OPTICAL_ZOOM,
                                      OLECMDEXECOPT_DONTPROMPTUSER,
                                      &vtIn,
                                      &vtOut );
    if (FAILED( hr )) {
        BPLOG_COM_ERROR( "set optical_zoom failed.", hr );
        return false;
    }

    BPLOG_INFO_STRM( "IE Zoom set to: " << nZoomPercent << "%." );
    return true;
}



} // namespace html
} // namespace bp


