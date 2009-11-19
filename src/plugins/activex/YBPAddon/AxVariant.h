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
 *  AxVariant.h
 *
 *  Created by David Grigsby on 10/24/2007.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef __AXVARIANT_H__
#define __AXVARIANT_H__

#include "BPUtils/bpstrutil.h"
#include "PluginCommonLib/PluginVariant.h"


class AxVariant : public plugin::Variant
{
public:
    AxVariant() {}

    AxVariant( CComVariant vt ) : m_data( vt ) {}
    
    ~AxVariant() {}

    ////////////////////////////
    // plugin::Variant Support
    //
    bool clear()
    {
        return SUCCEEDED( m_data.Clear() );
    }
    
    bool getBooleanValue( bool& bVal ) const
    {
        if (!isBoolean())
        {
            return false;
        }

        bVal = m_data.boolVal;
        return true;
    }
    
    bool getIntegerValue( int& nVal ) const
    {
        if (m_data.vt == VT_INT) nVal = m_data.intVal;
		else if (m_data.vt == VT_UI4) nVal = (int) m_data.ulVal;
		else if (m_data.vt == VT_I4) nVal = (int) m_data.lVal;
		else return false;

        return true;
    }
    
    bool getDoubleValue( double& dVal ) const
    {
        if (!isDouble())
        {
            return false;
        }

        dVal = m_data.dblVal;
        return true;
    }
    
    bool getStringValue( std::string& sVal ) const
    {
        if (!isString())
        {
            return false;
        }

        sVal = bp::strutil::wideToUtf8(std::wstring(_bstr_t(m_data.bstrVal)));
        return true;
    }
    
    bool getObjectValue( plugin::Object*& oVal ) const
    {
        if (!isObject())
        {
            return false;
        }

        oVal = new AxObject( CComPtr<IDispatch>( m_data.pdispVal ) );
        
        return true;
    }

    bool isVoid() const
    {
        return m_data.vt == VT_EMPTY;
    }
    
    bool isNull() const
    {
        return m_data.vt == VT_NULL;
    }
    
    bool isBoolean() const
    {
        return m_data.vt == VT_BOOL;
    }
    
    bool isInteger() const
    {
        return m_data.vt == VT_INT || m_data.vt == VT_UI4 || m_data.vt == VT_I4;
    }
    
    bool isDouble() const
    {
        return m_data.vt == VT_R8;
    }
    
    bool isString() const
    {
        return m_data.vt == VT_BSTR;
    }
    
    bool isObject() const
    {
        return m_data.vt == VT_DISPATCH;
    }
    

    // Accessors
    CComVariant& data() { return m_data; }
    
    const CComVariant& data() const { return m_data; }

    
private:
    CComVariant m_data;
};


#endif // __AXVARIANT_H__

