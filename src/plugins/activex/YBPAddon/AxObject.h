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
 *  AxObject.h
 *
 *  Created by David Grigsby on 10/24/2007.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef __AXOBJECT_H__
#define __AXOBJECT_H__

#include "PluginCommonLib/PluginObject.h"


class AxObject : public plugin::Object
{
public:
    AxObject() {}
    
    AxObject( CComPtr<IDispatch> disp ) : m_data( disp ) {}
    AxObject( const AxObject& rhs ) { m_data = rhs.m_data; }
    
    ~AxObject() {}

    // plugin::Object Support
    //
    plugin::Object* clone() const { return new AxObject( *this ); }

/*    
    // Methods specific to AxObject
    //
    bool enumerateProperties( std::vector<std::string>& vsProps ) const;
    
    bool getProperty( const std::string& sPropName,
                      CComVariant* pvtVal ) const;
*/
    
    // Accessors
    // NOTE: debate whether this should return val or ref.
    CComPtr<IDispatch> data() { return m_data; }
    const CComPtr<IDispatch> data() const { return m_data; } 
            
    
private:
    CComPtr<IDispatch> m_data;

    AxObject& operator=( const AxObject& );
};



#endif // __AXOBJECT_H__

