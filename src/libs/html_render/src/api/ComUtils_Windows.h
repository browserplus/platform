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
 *  ComUtils_Windows.h
 *
 *  COM utilities.
 *
 *  Created by David Grigsby on 10/10/2007.
 *  Original Author: Levi Wolfe
 *  
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef __COMTYPES_WINDOWS_H__
#define __COMTYPES_WINDOWS_H__

#include "BPUtils/BPLog.h"

#include <atlbase.h>
#include <comdef.h>
#include <dispex.h>
#include <sstream>

// Specialization for IDispatchEx.  Nearly identical to ATL's
// specialization for IDispatch except that GetIDOfName() is instead mapped
// to IDispatchEx::GetDispID() with flags to guarantee the
// member is created if it doesn't exist.
template <>
class CComPtr<IDispatchEx> : public CComPtrBase<IDispatchEx>
{
public:
    CComPtr() throw()
    {
    }
    CComPtr(IDispatchEx* lp) throw()
    :CComPtrBase<IDispatchEx>(lp)
    {
    }
    CComPtr(const CComPtr<IDispatchEx>& lp) throw()
    :CComPtrBase<IDispatchEx>(lp.p)
    {
    }
    IDispatchEx* operator=(IDispatchEx* lp) throw()
    {
        return static_cast<IDispatchEx*>(AtlComPtrAssign((IUnknown**)&p, lp));
    }
    IDispatchEx* operator=(const CComPtr<IDispatchEx>& lp) throw()
    {
        return static_cast<IDispatchEx*>(AtlComPtrAssign((IUnknown**)&p, lp.p));
    }
    // IDispatchEx specific stuff.
    HRESULT GetPropertyByName(LPCOLESTR lpsz, VARIANT* pVar) throw()
    {
        ATLASSERT(p);
        ATLASSERT(pVar);
        DISPID dwDispID;
        HRESULT hr = GetIDOfName(lpsz, &dwDispID);
        if (SUCCEEDED(hr))
        {
            hr = GetProperty(dwDispID, pVar);
        }
        return hr;
    }
    HRESULT GetProperty(DISPID dwDispID, VARIANT* pVar) throw()
    {
        return GetProperty(p, dwDispID, pVar);
    }
    HRESULT PutPropertyByName(LPCOLESTR lpsz, VARIANT* pVar) throw()
    {
        ATLASSERT(p);
        ATLASSERT(pVar);
        DISPID dwDispID;
        HRESULT hr = GetIDOfName(lpsz, &dwDispID);
        if (SUCCEEDED(hr))
        {
            hr = PutProperty(dwDispID, pVar);
        }
        return hr;
    }
    HRESULT PutProperty(DISPID dwDispID, VARIANT* pVar) throw()
    {
        return PutProperty(p, dwDispID, pVar);
    }
    HRESULT GetIDOfName(LPCOLESTR lpsz, DISPID* pdispid) throw()
    {
        CComBSTR bstr(lpsz);
        return p->GetDispID(bstr,
                            fdexNameCaseSensitive | fdexNameEnsure,
                            pdispid);
    }
    // Invoke a method by DISPID with no parameters.
    HRESULT Invoke0(DISPID dispid, VARIANT* pvarRet = NULL) throw()
    {
        DISPPARAMS dispparams = { NULL, NULL, 0, 0 };
        return p->Invoke(dispid,
                         IID_NULL,
                         LOCALE_USER_DEFAULT,
                         DISPATCH_METHOD,
                         &dispparams,
                         pvarRet,
                         NULL,
                         NULL);
    }
    // Invoke a method by name with no parameters.
    HRESULT Invoke0(LPCOLESTR lpszName, VARIANT* pvarRet = NULL) throw()
    {
        HRESULT hr;
        DISPID dispid;
        hr = GetIDOfName(lpszName, &dispid);
        if (SUCCEEDED(hr))
        {
            hr = Invoke0(dispid, pvarRet);
        }
        return hr;
    }
    // Invoke a method by DISPID with a single parameter.
    HRESULT Invoke1(DISPID dispid,
                    VARIANT* pvarParam1,
                    VARIANT* pvarRet = NULL) throw()
    {
        DISPPARAMS dispparams = { pvarParam1, NULL, 1, 0 };
        return p->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dispparams, pvarRet, NULL, NULL);
    }
    // Invoke a method by name with a single parameter
    HRESULT Invoke1(LPCOLESTR lpszName,
                    VARIANT* pvarParam1,
                    VARIANT* pvarRet = NULL) throw()
    {
        HRESULT hr;
        DISPID dispid;
        hr = GetIDOfName(lpszName, &dispid);
        if (SUCCEEDED(hr))
        {
            hr = Invoke1(dispid, pvarParam1, pvarRet);
        }
        return hr;
    }
    // Invoke a method by DISPID with two parameters.
    HRESULT Invoke2(DISPID dispid,
                    VARIANT* pvarParam1,
                    VARIANT* pvarParam2,
                    VARIANT* pvarRet = NULL) throw();
    // Invoke a method by name with two parameters.
    HRESULT Invoke2(LPCOLESTR lpszName,
                    VARIANT* pvarParam1,
                    VARIANT* pvarParam2,
                    VARIANT* pvarRet = NULL) throw()
    {
        HRESULT hr;
        DISPID dispid;
        hr = GetIDOfName(lpszName, &dispid);
        if (SUCCEEDED(hr))
        {
            hr = Invoke2(dispid, pvarParam1, pvarParam2, pvarRet);
        }
        return hr;
    }
    // Invoke a method by DISPID with N parameters.
    HRESULT InvokeN(DISPID dispid,
                    VARIANT* pvarParams,
                    int nParams,
                    VARIANT* pvarRet = NULL) throw()
    {
        DISPPARAMS dispparams = { pvarParams, NULL, nParams, 0};
        return p->Invoke(dispid,
                         IID_NULL,
                         LOCALE_USER_DEFAULT,
                         DISPATCH_METHOD,
                         &dispparams,
                         pvarRet,
                         NULL,
                         NULL);
    }
    // Invoke a method by name with N parameters.
    HRESULT InvokeN(LPCOLESTR lpszName,
                    VARIANT* pvarParams,
                    int nParams,
                    VARIANT* pvarRet = NULL) throw()
    {
        HRESULT hr;
        DISPID dispid;
        hr = GetIDOfName(lpszName, &dispid);
        if (SUCCEEDED(hr))
        {
            hr = InvokeN(dispid, pvarParams, nParams, pvarRet);
        }
        return hr;
    }
    static HRESULT PutProperty(IDispatchEx* p,
                               DISPID dwDispID,
                               VARIANT* pVar) throw()
    {
        ATLASSERT(p);
        ATLASSERT(pVar != NULL);
        if (pVar == NULL)
        {
            return E_POINTER;
        }
        if (p == NULL)
        {
            return E_INVALIDARG;
        }
        ATLTRACE(atlTraceCOM, 2, L"CPropertyHelper::PutProperty\n");
        DISPPARAMS dispparams = {NULL, NULL, 1, 1};
        dispparams.rgvarg = pVar;
        DISPID dispidPut = DISPID_PROPERTYPUT;
        dispparams.rgdispidNamedArgs = &dispidPut;
        if (pVar->vt == VT_UNKNOWN || pVar->vt == VT_DISPATCH || 
            (pVar->vt & VT_ARRAY) || (pVar->vt & VT_BYREF))
        {
            HRESULT hr = p->Invoke(dwDispID,
                                   IID_NULL,
                                   LOCALE_USER_DEFAULT,
                                   DISPATCH_PROPERTYPUTREF,
                                   &dispparams,
                                   NULL,
                                   NULL,
                                   NULL);
            if (SUCCEEDED(hr))
            {
                return hr;
            }
        }
        return p->Invoke(dwDispID,
                         IID_NULL,
                         LOCALE_USER_DEFAULT,
                         DISPATCH_PROPERTYPUT,
                         &dispparams,
                         NULL,
                         NULL,
                         NULL);
    }
    static HRESULT GetProperty(IDispatchEx* p,
                               DISPID dwDispID,
                               VARIANT* pVar) throw()
    {
        ATLASSERT(p);
        ATLASSERT(pVar != NULL);
        if (pVar == NULL)
        {
            return E_POINTER;
        }
        if (p == NULL)
        {
            return E_INVALIDARG;
        }
        ATLTRACE(atlTraceCOM, 2, L"CPropertyHelper::GetProperty\n");
        DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
        return p->Invoke(dwDispID,
                         IID_NULL,
                         LOCALE_USER_DEFAULT,
                         DISPATCH_PROPERTYGET,
                         &dispparamsNoArgs,
                         pVar,
                         NULL,
                         NULL);
    }
};

//typedef CComQIPtr<IDispatchEx, &__uuidof(IDispatchEx)> CComDispatchExDriver;


#define BPLOG_COM_ERROR( msg, hr )  \
BPLOG_ERROR_STRM( msg << " " << bp::com::comErrorString( (hr) ) )


#define BP_THROW_COM( sDesc, hr )                       \
{                                                       \
    bp::Exception e( (sDesc) );                         \
    ReportThrow( e, __FILE__, __BP_FUNC__, __LINE__,    \
                 bp::com::comErrorString( (hr) ) );     \
    throw e;                                            \
}


namespace bp {
namespace com {

// Return a string corresponding to 'hr'.
inline std::string comErrorString( HRESULT hr )
{
    std::stringstream ss;
    ss << "(" << hr << "): " << _com_error(hr).ErrorMessage();
    return ss.str();
}


inline bool addProperty( CComPtr<IDispatch> dispObj,
                         std::string sPropName,
                         CComVariant vtProp )
{
    // Get an IDispatchEx on the object.
    // Note our specialization of CComPtr<IDispatchEx> is needed for the call to
    // PutPropertyByName to work correctly.
    CComQIPtr<IDispatchEx> dexObj( dispObj );

    // Add the property to the obj.
    HRESULT hr = dexObj.PutPropertyByName( _bstr_t( sPropName.c_str() ),
                                           &vtProp );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "dexObj.PutPropertyByName() failed!", hr );
        return false;
    }

    return true;
}


} // namespace com
} // namespace bp




#endif // __COMUTILS_WINDOWS_H__

