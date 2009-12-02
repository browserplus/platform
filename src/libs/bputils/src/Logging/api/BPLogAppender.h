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
 *  BPLogAppender.h
 *
 *  Created by David Grigsby on 9/19/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _LOGAPPENDER_H_
#define _LOGAPPENDER_H_

#include <vector>

#include "BPUtils/BPLogEvent.h"
#include "BPUtils/BPLogLayout.h"
#include "BPUtils/bpsync.h"


namespace bp {
namespace log {


class Appender
{
public:
//  Appender();
    Appender( LayoutPtr layout );
    virtual ~Appender();

    void doAppend( const LoggingEventPtr& evt );

    void setThreshold( const Level& threshold );

    void setLayout( const LayoutPtr& layout );

    
protected:
    virtual void append( LoggingEventPtr evt ) = 0;
    Level       m_threshold;
    LayoutPtr   m_layout;   
    bp::sync::Mutex m_mutex;

    
private:
    Appender( const Appender& );
    Appender& operator=( const Appender& );
};

typedef std::tr1::shared_ptr<Appender> AppenderPtr;

typedef std::vector<AppenderPtr>    AppenderList;
typedef AppenderList::iterator      AppenderListIt;


} // bp
} // log


#endif // _LOGAPPENDER_H
