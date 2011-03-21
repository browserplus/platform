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
 *  bperrorutil.cpp
 *
 *  Created by David Grigsby on 8/29/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include "bperrorutil.h"

#include <sstream>

#ifdef WINDOWS
#pragma warning(disable: 4995)
#include <windows.h>
#include <strsafe.h>
#else
#include <stdio.h>
#include <errno.h>
#include <string.h>
#endif

#include "BPLog.h"
#include "bpstrutil.h"

using namespace std;


namespace bp {
namespace error {


string lastErrorString(const char * contextStr)
{
    stringstream ss;

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


string lastErrorString(const string& contextStr)
{
    return lastErrorString(contextStr.c_str());
}


Exception::Exception( const string& sDesc ) :
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
    

const char* Exception::what() const throw()
{
    return m_sDesc.c_str();
}


FatalException::FatalException( const string& sDesc ) :
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
    
    
const char* FatalException::what() const throw()
{
    return m_sDesc.c_str();
}


static string
makeExceptionEventString( const string sAction,
                          const exception& exc,
                          const string& sAddlContext )
{
    stringstream ssReport;
    ssReport << sAction << " a " << typeid(exc).name() << ": " << exc.what();
    if (!sAddlContext.empty()) {
        ssReport << " (" << sAddlContext << ")";
    }

    return ssReport.str();
}


static void
reportExceptionEvent( const string sAction,
                      const exception& e,
                      const string& sFile,
                      const string& sFunc,
                      int nLine,
                      const string& sAddlContext ) throw()
{
    using namespace bp::log;

    const Level kExcLogLevel = LEVEL_ERROR;
    
    if (rootLogger().isLevelEnabled( kExcLogLevel ))
    {
        string sLog = makeExceptionEventString( sAction, e, sAddlContext );
        LocationInfo loc( sFile, sFunc, nLine );
        rootLogger()._forcedLog( kExcLogLevel, sLog, loc );
    }
}


void reportThrow( const exception& e,
                  const string& sFile,
                  const string& sFunc,
                  int nLine,
                  const string& sAddlContext )
{
    reportExceptionEvent( "Throwing", e, sFile, sFunc, nLine, sAddlContext );
}


void reportCatch( const exception& e,
                  const string& sFile,
                  const string& sFunc,
                  int nLine,
                  const string& sAddlContext ) throw()
{
    reportExceptionEvent( "Caught", e, sFile, sFunc, nLine, sAddlContext );
}


void reportCatchUnknown( const string& sFile,
                         const string& sFunc,
                         int nLine,
                         const string& sAddlContext ) throw()
{
    using namespace bp::log;

    const Level kExcLogLevel = LEVEL_ERROR;

    if (rootLogger().isLevelEnabled( kExcLogLevel ))
    {
        stringstream ssReport;
        ssReport << "Caught an unknown exception.";
        if (!sAddlContext.empty()) {
            ssReport << " (" << sAddlContext << ")";
        }

        LocationInfo loc( sFile, sFunc, nLine );
        rootLogger()._forcedLog( kExcLogLevel, ssReport.str(), loc );
    }
}



// These next two are for external users who want the raw string,
// (e.g. for sending to cout).

string makeThrowReportString( const exception& exc,
                              const string& sAddlContext )
{
    return makeExceptionEventString( "Throwing", exc, sAddlContext );
}


string makeCatchReportString( const exception& exc,
                              const string& sAddlContext )
{
    return makeExceptionEventString( "Caught", exc, sAddlContext );
}


} // namespace error
} // namespace bp
