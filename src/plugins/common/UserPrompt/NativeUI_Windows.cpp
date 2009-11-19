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
 * Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/*
 *  NativeUI_Windows.cpp
 *
 *  Implements the native windows user interface functions for
 *  BrowserPlus.
 *  
 *  Created by David Grigsby
 *  Copyright 2007-2008 Yahoo! Inc. All rights reserved.
 */
#include "NativeUI.h"
#include "HTMLRender/HTMLDialog_Windows.h"
#include "HTMLRender/HTMLScriptObject.h"
#include "HTMLRender/ScriptGateway_Windows.h"

#include "windows.h"

using namespace bp::html;
using namespace std;


//////////////////////////////////////////////////////////////////////
// PromptApiProvider

class PromptApiProvider : public ScriptableFunctionHost
{
// Construction/Destruction    
public:
    PromptApiProvider( const bp::Object* pArgs, bp::Object** ppResponse );
    virtual ~PromptApiProvider();

// Methods
public:
    virtual bp::Object* invoke( const std::string& functionName,
                                unsigned int id,
                                vector<const bp::Object*> args );

// Accessors
public:
    const ScriptableObject& getScriptableObject()   { return m_so; }
    void setDialog( HtmlDialog& dlg )               { m_pDlg = &dlg; }
    
// State
private:
    const bp::Object*   m_pArgs;
    bp::Object**        m_ppResponse;
    ScriptableObject    m_so;
    HtmlDialog*         m_pDlg;
    
// Prevent copying
private:
    PromptApiProvider( const PromptApiProvider& );
    PromptApiProvider& operator=( const PromptApiProvider& );
};



PromptApiProvider::PromptApiProvider( const bp::Object* pArgs,
                                      bp::Object** ppResponse ) :
    m_pArgs( pArgs ),
    m_ppResponse( ppResponse ),
    m_pDlg( 0 )
{
    // Our "scriptable object" will direct calls to these methods to
    // our invoke().
    m_so.mountFunction( this, "args" );
    m_so.mountFunction( this, "dpiHack" );
    m_so.mountFunction( this, "complete" );
    m_so.mountFunction( this, "log" );
    m_so.mountFunction( this, "show" );
}


PromptApiProvider::~PromptApiProvider()
{
}


bp::Object* PromptApiProvider::invoke( const std::string& sFuncName,
                                       unsigned int /*nId*/,
                                       vector<const bp::Object*> vArgs )
{
    // TODO: should probably use BP_THROW
    
    if (sFuncName == "log")
    {
        for (unsigned int i = 0; i < vArgs.size(); i++)
        {
            std::string s;
            if (vArgs[i]->type() == BPTString)
            {
                s = std::string(*(vArgs[i]));
            }
            else
            {
                s = vArgs[i]->toPlainJsonString(true);
            }

            BPLOG_INFO_STRM("JavaScript Logging: " << s);
        }

        return new bp::Null;
    }
    else if (sFuncName == "complete")
    {
        if (vArgs.size() != 1)
        {
            throw std::string( "complete function takes exactly one argument" );
        }

        if (!m_pDlg)
        {
            throw std::string( "no dialog was set for provider" );
        }

        *m_ppResponse = vArgs[0]->clone();
        m_pDlg->EndDialog( 1 );
        
        return new bp::Null;
    }
    else if (sFuncName == "args")    
    {
        if (vArgs.size() != 0)
        {
            throw std::string( "args function takes no arguments" );
        }
        
        return m_pArgs->clone();
    }
    else if (sFuncName == "dpiHack")
    {
        bool dpiHack = true;
        if (m_pDlg && m_pDlg->dpiAware()) dpiHack = false;
        return new bp::Bool(dpiHack);
    }
    else if (sFuncName == "show")
    {
        if (vArgs.size() != 2 ||
            vArgs[0]->type() != BPTInteger ||
            vArgs[1]->type() != BPTInteger)
        {
            throw std::string( "invalid arguments" );
        }
        
        int nWidth  = (int) (long long) *(vArgs[0]);
        int nHeight = (int) (long long) *(vArgs[1]);

        // Handle high DPI on windows:  When our UI is rendered on
        // a windows host with IE8 installed (high dpi aware HTML rendering),
        // then we must scale our container up based on DPI settings.
        // (YIB-2858594)
        float scale = 1.0;

        if (m_pDlg && m_pDlg->dpiAware())
        {
            scale = m_pDlg->dpiScale();
        }
            
        if (!m_pDlg)
        {
            throw std::string( "no dialog was set for provider" );
        }

        m_pDlg->show( (int) (nWidth * scale), 
                      (int) (nHeight * scale) );

        return new bp::Null;
    }
    else
    {
        throw std::string( "unrecognized invoke function name" );
        return NULL;
    }
}



//////////////////////////////////////////////////////////////////////
// HTMLPrompt

bool
bp::ui::HTMLPrompt(void* pvParentWnd,
                   const std::string& sPathToHtml,
                   const std::string& /*sUserAgent*/,
                   const bp::Object* pArgs,
                   bp::Object** ppResponse)
{
    try
    {
        PromptApiProvider provider( pArgs, ppResponse );
        ScriptGateway gateway( provider.getScriptableObject(), "BPDialog" );

        HICON hIcon = 0;

        HtmlDialog dlg( "BrowserPlus", sPathToHtml, hIcon, &gateway );
        provider.setDialog( dlg );

        dlg.DoModal( (HWND) pvParentWnd );

        return true;
    }
    catch ( bp::error::Exception& exc )
    {
        BP_REPORTCATCH( exc );
        
        // TODO: localize
        // TODO: standard skinned ok box method
        MessageBoxW( (HWND) pvParentWnd, L"Unable to display prompt.",
                     L"BrowserPlus",  MB_OK );

        return false;
    }

    // TODO: might want to catch std::exception and ...
}
