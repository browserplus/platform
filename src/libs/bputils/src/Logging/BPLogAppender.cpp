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
 *  BPLogAppender.cpp
 *
 *  Created by David Grigsby on 9/18/07.
 *  Portions based on code in log4cxx.
 *  
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include "BPLogAppender.h"


namespace bp {
namespace log {


Appender::Appender( LayoutPtr layout ) :
    m_threshold( LEVEL_ALL ),
    m_layout( layout )
{

}


Appender::~Appender()
{

}


void Appender::setThreshold( const Level& threshold )
{
    m_threshold = threshold;
}


void Appender::setLayout( const LayoutPtr& layout )
{
    m_layout = layout;
}


void Appender::doAppend( const LoggingEventPtr& event )
{
    // TODO: Autolock?
    m_mutex.lock();
    
    if (event->level() < m_threshold)
    {
        m_mutex.unlock();
        return;
    }

    // TODO: check appender closed?
    append( event );

    m_mutex.unlock();
}


} // log
} // bp
