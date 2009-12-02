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
 *  bperrorutil.cpp
 *
 *  Created by David Grigsby on 8/29/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include "bperrorutil.h"

#include <sstream>

#ifdef WINDOWS
#include <windows.h>
#include <strsafe.h>
#else
#include <stdio.h>
#include <errno.h>
#endif

#include "BPLog.h"
#include "bpstrutil.h"


namespace bp {
namespace error {

std::string 
lastErrorString(const char * contextStr)
{
    std::stringstream ss;

    if (NULL != contextStr) ss << contextStr << " ";

#ifdef WINDOWS
    LPTSTR msg;
    DWORD dw = GetLastError(); 
    DWORD res = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                              FORMAT_MESSAGE_FROM_SYSTEM |
                              FORMAT_MESSAGE_IGNORE_INSERTS,
                              NULL,
                              dw,
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                              (LPTSTR)&msg,
                              0, NULL );

    ss << "(" << dw << ")";
    
    if (res) {
        ss << ": " << bp::strutil::wideToUtf8(msg);
        LocalFree(msg);
    }
#else
    ss << "(" << errno << "): " << strerror(errno);
#endif
    return ss.str();
}


std::string
lastErrorString(const std::string& contextStr)
{
    return lastErrorString(contextStr.c_str());
}


Exception::Exception( const std::string& sDesc ) :
m_sDesc( sDesc )
{
}

    
Exception::Exception( const Exception& other ) :
m_sDesc( other.m_sDesc )
{
}
    

Exception::~Exception() throw()
{

}


Exception& Exception::operator=( const Exception& other )
{
    m_sDesc = other.m_sDesc;
    return *this;
}
    
    
Exception* Exception::clone() const
{
    return new Exception( *this );
}
    

const char*
Exception::what() const throw()
{
    return m_sDesc.c_str();
}


FatalException::FatalException( const std::string& sDesc ) :
m_sDesc( sDesc )
{
}


FatalException::FatalException( const FatalException& other ) :
m_sDesc( other.m_sDesc )
{
}

    
FatalException::~FatalException() throw()
{

}


FatalException& FatalException::operator=( const FatalException& other )
{
    m_sDesc = other.m_sDesc;
    return *this;
}
    
    
FatalException* FatalException::clone() const
{
    return new FatalException( *this );
}
    
    
const char*
FatalException::what() const throw()
{
    return m_sDesc.c_str();
}


void ReportThrow( const std::exception& e,
                  const std::string& sFile,
                  const std::string& sFunc,
                  int nLine,
                  const std::string& sAddlContext )
{
    std::stringstream ssReport;
    ssReport << "Throwing a " << typeid(e).name() << ": " << e.what();
    if (!sAddlContext.empty())
    {
        ssReport << "info: " << sAddlContext;
    }
    
    if (bp::log::rootLogger().isLevelEnabled( bp::log::LEVEL_ERROR ))
    {
        bp::log::rootLogger()._forcedLog( bp::log::LEVEL_ERROR, ssReport.str(),
                                          bp::log::LocationInfo( sFile,
                                                                 sFunc,
                                                                 nLine ) );
    }
}


void
ReportCatch( const std::exception& e,
             const std::string& sFile,
             const std::string& sFunc,
             int nLine,
             const std::string& sAddlContext ) throw()
{
    if (bp::log::rootLogger().isLevelEnabled(bp::log::LEVEL_ERROR))
    {
        bp::log::rootLogger()._forcedLog(bp::log::LEVEL_ERROR,
                                         makeCatchReportString(e, sAddlContext),
                                         bp::log::LocationInfo(sFile,
                                                               sFunc,
                                                               nLine));
    }
}


std::string makeCatchReportString( const std::exception& exc,
                                   const std::string& sAddlContext )
{
    std::stringstream ssReport;
    ssReport << "Caught a " << typeid(exc).name() << ": " << exc.what();
    if (!sAddlContext.empty())
    {
        ssReport << "info: " << sAddlContext;
    }

    return ssReport.str();
}


} // namespace error
} // namespace bp
