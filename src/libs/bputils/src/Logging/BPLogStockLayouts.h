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
 *  BPLogStockLayouts.h
 *
 *  Declares built-in layouts.
 *  
 *  Created by David Grigsby on 6/16/09.
 *  
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _BPLOGSTOCKLAYOUTS_H_
#define _BPLOGSTOCKLAYOUTS_H_

#include <string>
#include "BPLogLayout.h"


namespace bp {
namespace log {


class RawLayout : public Layout
{
public:    
    RawLayout() {}
    ~RawLayout() {}

    virtual void format( std::string& sOut, const LoggingEventPtr& event );
};


class SourceLayout : public Layout
{
public:    
    SourceLayout() {}
    ~SourceLayout() {}

    virtual void format( std::string& sOut, const LoggingEventPtr& event );
};


class StandardLayout : public Layout
{
public:    
    StandardLayout() {}
    ~StandardLayout() {}

    virtual void format( std::string& sOut, const LoggingEventPtr& event );
};


class ThrdLvlFuncMsgLayout : public Layout
{
public:    
    ThrdLvlFuncMsgLayout() {}
    ~ThrdLvlFuncMsgLayout() {}

    virtual void format( std::string& sOut, const LoggingEventPtr& event );
};


class TimeLvlMsgLayout : public Layout
{
public:    
    TimeLvlMsgLayout() {}
    ~TimeLvlMsgLayout() {}

    virtual void format( std::string& sOut, const LoggingEventPtr& event );
};


class FuncMsgLayout : public Layout
{
public:    
    FuncMsgLayout() {}
    ~FuncMsgLayout() {}

    virtual void format( std::string& sOut, const LoggingEventPtr& event );
};



} // log
} // bp


#endif // _BPLOGSTOCKLAYOUTS_H
