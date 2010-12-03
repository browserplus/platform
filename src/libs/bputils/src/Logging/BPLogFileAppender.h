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
 *  BPLogFileAppender.h
 *
 *  Created by David Grigsby on 9/19/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef _BPLOGFILEAPPENDER_H_
#define _BPLOGFILEAPPENDER_H_

#include <fstream>
#include <string>
#include "bpfile.h"
#include "BPLogAppender.h"
#include "BPLogFile.h"


namespace bp {
namespace log {


class FileAppender : public Appender
{
public:
    FileAppender( const boost::filesystem::path& path,
                  LayoutPtr layout,
                  FileMode mode,
                  unsigned int nRolloverSizeKB,
                  bool bImmediateFlush=true );

    virtual ~FileAppender();

    virtual void append( LoggingEventPtr evt );
    
private:
    // path to log file
    boost::filesystem::path  m_path;

    // file stream - stays open between appends
    std::ofstream   m_fstream;

    // truncate/append/rollover mode
    FileMode        m_mode;
    
    // used when mode==kSizeRollover
    // rollover check is performed at first append
    // existing file is emptied if it exceeds rollover size
    unsigned int    m_nRolloverSizeKB;
    
    // whether to flush after each append
    bool            m_bImmediateFlush;

private:
    FileAppender( const FileAppender& );
    FileAppender& operator=( const FileAppender& );
};


typedef std::tr1::shared_ptr<FileAppender> FileAppenderPtr;


} // bp
} // log

#endif // _BPLOGFILEAPPENDER_H
