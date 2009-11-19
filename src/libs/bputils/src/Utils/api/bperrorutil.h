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
 * Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/*
 *  bperrorutil.h
 *
 *  Created by David Grigsby on 8/29/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef __BPERRORUTIL_H__
#define __BPERRORUTIL_H__

#include <exception>
#include <string>

namespace bp {
namespace error {


/**
 * return a formated error message which includes both the decimal
 * representation and the human readable description of the last
 * system call error.
 *
 * example return:  "your context string (23): Invalid file descriptor"
 *
 * contextStr if provided will be prepended to the formatted message
 * and allows the caller to conveniently embed a little context.
 */
std::string lastErrorString(const char * contextStr = NULL);
std::string lastErrorString(const std::string& contextStr);

class Exception : public std::exception
{
public:
    explicit Exception( const std::string& sDesc );
    Exception( const Exception& other );
    virtual ~Exception() throw();
    Exception& operator=( const Exception& other );
    virtual Exception* clone() const;  // caller owns returned value
    virtual const char* what() const throw();

protected:
    std::string m_sDesc;
};

/**
 * A FatalException is just that, fatal.  Don't try to catch and recover.  At 
 * most, catch, log, and die/disable.
 */
class FatalException : public std::exception
{
public:
    explicit FatalException( const std::string& sDesc );
    FatalException( const FatalException& other );
    virtual ~FatalException() throw();
    FatalException& operator=( const FatalException& other );
    virtual FatalException* clone() const;  // caller owns returned value
    virtual const char* what() const throw();

protected:
    std::string m_sDesc;
};



void ReportThrow( const std::exception& e,
                  const std::string& sFile,
                  const std::string& sFunc,
                  int nLine,
                  const std::string& sAddlContext="" );


#if defined(_MSC_VER)
  #if _MSC_VER >= 1300
    #define __BP_PRETTY_FUNC__ __FUNCSIG__
    #define __BP_FUNC__ __FUNCTION__
  #endif
#else
  #if defined(__GNUC__)
    #define __BP_PRETTY_FUNC__ __PRETTY_FUNCTION__
    #define __BP_FUNC__ __FUNCTION__
  #endif
#endif
#if !defined(__BP_FUNC__)
  #define __BP_PRETTY_FUNC__ "<unknown>"
  #define __BP_FUNC__ "<unknown>"
#endif



#define BP_THROW( sDesc )                               \
{                                                       \
    bp::error::Exception e( (sDesc) );                  \
    ReportThrow( e, __FILE__, __BP_FUNC__, __LINE__ );  \
    throw e;                                            \
}

#define BP_THROW_FATAL( sDesc )                         \
{                                                       \
    bp::error::FatalException e( (sDesc) );             \
    ReportThrow( e, __FILE__, __BP_FUNC__, __LINE__ );  \
    throw e;                                            \
}

#define BP_THROW_TYPE( type, sDesc )                    \
{                                                       \
    type e( (sDesc) );                                  \
    ReportThrow( e, __FILE__, __BP_FUNC__, __LINE__ );  \
    throw e;                                            \
}


// Report to the log system that an exception was caught.
void ReportCatch( const std::exception& e,
                  const std::string& sFile,
                  const std::string& sFunc,
                  int nLine,
                  const std::string& sAddlContext="") throw();


#define BP_REPORTCATCH(e)                               \
bp::error::ReportCatch( (e), __FILE__, __BP_FUNC__, __LINE__ );  


// Returns a string describing caught exception.
std::string makeCatchReportString(const std::exception& exc,
                                  const std::string& sAddlContext="" );

} // namespace error
} // namespace bp

#endif
