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
 *  NPAPIVariant.h
 *
 *  Wraps an NPVariant * in dave's cross browser abstraction
 *  
 *  Created by Lloyd Hilaiel on or around Thu Nov 15 20:35:21 PST 2007
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef __NPAPIVARIANT_H__
#define __NPAPIVARIANT_H__

#include <npapi/npapi.h>
#include <npapi/npruntime.h>
#include <npapi/npupp.h>

#include "nputils.h"

#include "PluginCommonLib/PluginVariant.h"
#include "NPAPIObject.h"

class NPAPIVariant : public plugin::Variant
{
  public:
    NPAPIVariant() {
        memset((void *) &m_var, 0, sizeof(m_var));
        m_var.type = NPVariantType_Void;
    }

    NPAPIVariant(NPVariant * var) {
        copyFrom(var);
    }
    
    ~NPAPIVariant() { }
    

    virtual bool clear() 
    {
        // TODO: not yet implemented!
        return false;
    }
        
    virtual bool getBooleanValue( bool& bVal) const
    {
        if (!isBoolean()) return false;
        bVal = (bool) NPVARIANT_TO_BOOLEAN(m_var);
        return true;
    }
    
    virtual bool getIntegerValue( int& nVal) const {    
        if (!isInteger()) return false;
        nVal = NPVARIANT_TO_INT32(m_var);
        return true;
    }
    
    virtual bool getDoubleValue( double& dVal ) const
    {
        if (!isDouble()) return false;
        dVal = NPVARIANT_TO_DOUBLE(m_var);
        return true;
    }
    
    virtual bool getStringValue( std::string& sVal ) const
    {
        sVal.clear();
        if (!isString()) return false;
        NPString tmpStr = NPVARIANT_TO_STRING(m_var);
        sVal.append(static_cast<const char*>(tmpStr.utf8characters),
                    tmpStr.utf8length);
        return true;
    }

    virtual bool getObjectValue( plugin::Object*& oVal ) const
    {
        if (!isObject()) return false;
        oVal = new NPAPIObject(m_var.value.objectValue);
        return true;
    }
    
    virtual bool isVoid() const 
    {
        return NPVARIANT_IS_VOID(m_var);
    }
    
    virtual bool isNull() const 
    {
        return NPVARIANT_IS_NULL(m_var);
    }
    
    virtual bool isBoolean() const
    {
        return NPVARIANT_IS_BOOLEAN(m_var);
    }

    virtual bool isInteger() const
    {
        return NPVARIANT_IS_INT32(m_var);
    }

    virtual bool isDouble() const
    {
        return NPVARIANT_IS_DOUBLE(m_var);
    }

    virtual bool isString() const
    {
        return NPVARIANT_IS_STRING(m_var);
    }

    virtual bool isObject() const
    {
        return NPVARIANT_IS_OBJECT(m_var);
    }

    NPVariant * varPtr() { return &m_var; };
    
    void copyTo(NPVariant * target) 
    {
        memcpy((void *) target, (void *) &m_var, sizeof(m_var));
    }

    void copyFrom(const NPVariant * source) 
    {
        memcpy((void *) &m_var, (void *) source, sizeof(m_var));
    }

  private:
    NPVariant m_var;
};

#endif
