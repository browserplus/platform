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
 *  PluginVariant.h
 *
 *  Declares the abstract plugin::Variant class.
 *  
 *  Created by David Grigsby on 10/22/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef __PLUGINVARIANT_H__
#define __PLUGINVARIANT_H__


// Forward decls
namespace plugin
{
    class Object;
}


namespace plugin {

class Variant
{
public:
    Variant() {}
    virtual ~Variant() {}
    
    virtual bool clear() = 0;
    
    virtual bool getBooleanValue( bool& bVal) const = 0;
    virtual bool getIntegerValue( int& nVal) const = 0;
    virtual bool getDoubleValue( double& dVal ) const = 0;
    virtual bool getStringValue( std::string& sVal ) const = 0;

    // TODO: change this to take a plugin::Object&?
    virtual bool getObjectValue( plugin::Object*& oVal ) const = 0;
    
    virtual bool isVoid() const = 0;
    virtual bool isNull() const = 0;
    virtual bool isBoolean() const = 0;
    virtual bool isInteger() const = 0;
    virtual bool isDouble() const = 0;
    virtual bool isString() const = 0;
    virtual bool isObject() const = 0;

private:
    Variant( const Variant& );
    Variant& operator=( const Variant& );
};


} // plugin


#endif // __PLUGINVARIANT_H__
