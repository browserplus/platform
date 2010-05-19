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
 *  HTMLScriptableComObject_Windows.h
 *
 *  Declares a special bp com object suitable for interaction with script.
 *
 *  Created by David Grigsby on 9/12/08.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _SCRIPTABLECOMOBJECT_H_
#define _SCRIPTABLECOMOBJECT_H_

#include <atlbase.h>
#include <atlcom.h>
#include <exdisp.h>
#include <exdispid.h>


// Forward Decls
namespace bp { namespace html { class ScriptableObject; } }


// IBpScriptableObject
[
    object,
    uuid("525B842B-44E6-4aa1-AF8C-9189870A8AA9"),
    dual,
    helpstring("IBpScriptableObject Interface"),
    pointer_default(unique)
]
__interface IBpScriptableObject : IDispatch
{
    [id(1), helpstring("method _bpInvoke")]
            HRESULT _bpInvoke([in] BSTR bstrMethodName,
                              [in] VARIANT vtArgs,
                              [out,retval] VARIANT* pvtResult );
};


// BpScriptableComObject
[
    coclass,
    default("IBpScriptableObject"),
    threading(apartment),
    vi_progid("YBPAddon.BpScriptableObject"),
    progid("YBPAddon.BpScriptableObject.1"),
    version(1.0),
    uuid("E58CF3D5-131E-4122-8079-F4A5C8674FFD"),
    noncreatable,
    aggregatable(never),
    helpstring("BpScriptableObject Class")
]
class ATL_NO_VTABLE BpScriptableComObject :
    public IBpScriptableObject
{
// Class factory
public:
//  static CComObject<BpScriptableComObject> create();
    
        
// Construction/destruction    
public:
    BpScriptableComObject() {}

    void FinalRelease() {}

    void setScriptableObject( const bp::html::ScriptableObject* pSO );
    void setBrowser( const CComPtr<IWebBrowser2>& browser );

    
// IBpScriptableObject methods    
public:
    STDMETHOD(_bpInvoke)( BSTR bstrName, VARIANT vtArgs, VARIANT* pvtResult );

    
// State    
private:
    const bp::html::ScriptableObject*   m_pSO;
    CComPtr<IWebBrowser2>               m_browser;
};


#endif
