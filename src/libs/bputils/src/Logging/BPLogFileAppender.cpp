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
 *  BPLogFileAppender.cpp
 *
 *  Created by David Grigsby on 9/19/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include "BPLogFileAppender.h"
#include "bpfile.h"


#ifdef WIN32
#define BP_OS_LINEEND "\r\n"
#else
#define BP_OS_LINEEND "\n"
#endif


namespace bp {
namespace log {


FileAppender::FileAppender( const bp::file::Path& path,
                            LayoutPtr layout,
                            bool bTruncateExisting,
                            bool bImmediateFlush ) :
    Appender( layout ),
    m_path( path ),
    m_bTruncateExisting( bTruncateExisting ),
    m_bImmediateFlush( bImmediateFlush )
{

}


FileAppender::~FileAppender()
{

}


void FileAppender::append( LoggingEventPtr event )
{
    // If we've had a failure with our fstream, go no further.
    if (m_fstream.fail())
    {
        return;
    }

    if (!m_fstream.is_open())
    {
        if (m_bTruncateExisting) {
            // TODO: could possibly assert here.
            (void) bp::file::remove( m_path );
        }
        
        std::ios_base::openmode mode = std::ios::binary | std::ios::app;
        
		if (!bp::file::openWritableStream( m_fstream, m_path, mode ))
        {
            // TODO: could possibly assert here.
            return;
        }
    }

    // Make the log string.
    std::string sMsg;
    m_layout->format( sMsg, event );
    sMsg.append( BP_OS_LINEEND );
    
    // Write it.
    m_fstream.write( sMsg.c_str(), static_cast<std::streamsize>(sMsg.length()));
    if (m_bImmediateFlush)
    {
        m_fstream.flush();
    }
}


} // log
} // bp
