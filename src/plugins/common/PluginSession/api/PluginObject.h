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
 *  PluginObject.h
 *
 *  Declares the abstract plugin::Object class.
 *  
 *  Created by David Grigsby on 10/22/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef __PLUGINOBJECT_H__
#define __PLUGINOBJECT_H__


namespace plugin {

class Object
{
public:
    Object() {}
    virtual ~Object() {}
            
    virtual Object * clone() const = 0;
    
private:
    Object( const Object& );
    Object& operator=( const Object& );
};

} // plugin


#endif // __PLUGINOBJECT_H__
