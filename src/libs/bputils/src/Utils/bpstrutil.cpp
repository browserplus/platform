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
 *  bpstrutil.cpp
 *
 *  Created by Gordon Durand on 7/20/07.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#include "bpstrutil.h"

#include <algorithm>
#include <assert.h>
#include <ios>
#include <iostream>
#include <iterator>
#include <fstream>
#include <functional>

#ifdef WIN32
#include "windows.h"
#endif
#ifdef MACOSX
#include <Carbon/Carbon.h>
#endif

//#include "boost/algorithm/string/split.hpp"

#include "bperrorutil.h"
#include "bpfile.h"
#include "bptr1.h"


namespace bp {
namespace strutil {


std::wstring
utf8ToWide(const std::string& sIn)
{
    std::wstring rval;
#ifdef WIN32
    // See how much space we need.
    int nChars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, sIn.c_str(), 
                                     -1, 0, 0);

    // Do the conversion.
    wchar_t* pawBuf = new wchar_t[nChars];
    int nRtn = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, sIn.c_str(), 
                                   -1, pawBuf, nChars);
    if (nRtn==0) {
        delete[] pawBuf;
        std::string sErr = "MultiByteToWideChar returned: " +
                           bp::error::lastErrorString();
        BP_THROW( sErr );
    } else {
        rval = pawBuf;
        delete[] pawBuf;
    }
#elif defined(MACOSX)
    CFStringRef cfStr = CFStringCreateWithBytes(NULL,
                                reinterpret_cast<const UInt8*>(sIn.c_str()),
                                sIn.length(),
                                kCFStringEncodingUTF8,
                                false);
    CFIndex cfLen = CFStringGetLength(cfStr);
    if (cfLen > 0) {
        UniChar* wstr = new UniChar[cfLen+1];
        CFRange r = { 0, cfLen };
        CFStringGetCharacters(cfStr, r, wstr);
        wstr[cfLen] = 0;
        rval.assign(reinterpret_cast<wchar_t*>(wstr), cfLen);
        delete[] wstr;
    } else {
        rval.clear();
    }
    if (cfStr) {
        CFRelease(cfStr);
    }
#elif defined(LINUX)
    assert("utf8ToWide not implemented on linux" == NULL);
#else
    unsupported platform;
#endif
    return rval;
}


std::string
wideToUtf8(const std::wstring& wsIn)
{
    std::string rval;
#ifdef WIN32
    // See how much space we need.
    // TODO: On Vista, it might be nice to specify WC_ERR_INVALID_CHARS
    int nChars = WideCharToMultiByte(CP_UTF8, 0, wsIn.c_str(), -1,
                                     0, 0, 0, 0);

    // Do the conversion.
    char* paBuf = new char[nChars];
    int nRtn = WideCharToMultiByte(CP_UTF8, 0, wsIn.c_str(), -1,
                                   paBuf, nChars, 0, 0);

    if (nRtn==0) {
        delete[] paBuf;
        std::string sErr = "WideCharToMultiByte returned: " +
                           bp::error::lastErrorString();
        BP_THROW( sErr );
    } else {
        rval = paBuf;
        delete[] paBuf;
    }
#elif defined(MACOSX)
	const UniChar* wstr = reinterpret_cast<const UniChar*>(wsIn.data());
	size_t len = wsIn.length();
	CFStringRef theString =
        CFStringCreateWithBytes(NULL, (const UInt8 *) wstr,
                                (len+1) * sizeof(wchar_t),
                                kCFStringEncodingUTF32LE,
                                false);

	CFIndex bufSize = CFStringGetMaximumSizeForEncoding(len, 
                                                        kCFStringEncodingUTF8);
	char* cstr = new char[bufSize+1];
	if (CFStringGetCString(theString, cstr, 
                           bufSize+1, kCFStringEncodingUTF8)) {
        rval.assign(cstr, strlen(cstr));
    } else {
        rval.clear();
	}
    delete[] cstr;
	if (theString) {
		CFRelease(theString);
    }
#elif defined(LINUX)
	const wchar_t * wstr = reinterpret_cast<const wchar_t*>(wsIn.data());
    assert(sizeof(wchar_t) == 4);
    while (*wstr) {
        wchar_t codepoint = *wstr++;
        if (codepoint < 0x80) {
            rval.append(1, (char) codepoint);
        } else if (codepoint < 0x0800) {
            rval.append(1, (char) ((codepoint >> 6) | 0xC0));
            rval.append(1, (char) ((codepoint & 0x3F) | 0x80));
        } else if (codepoint < 0x10000) {
            rval.append(1, (char) ((codepoint >> 12) | 0xE0));
            rval.append(1, (char) (((codepoint >> 6) & 0x3F) | 0x80));
            rval.append(1, (char) ((codepoint & 0x3F) | 0x80));
        } else if (codepoint < 0x200000) {
            rval.append(1, (char)((codepoint >> 18) | 0xF0));
            rval.append(1, (char)(((codepoint >> 12) & 0x3F) | 0x80));
            rval.append(1, (char)(((codepoint >> 6) & 0x3F) | 0x80));
            rval.append(1, (char)((codepoint & 0x3F) | 0x80));
        } else {
            rval.append("?");                 
        }
    }
#else
    unsupported platform;
#endif
    return rval;
}



void
replace(std::string& sTarget,
        const std::string& sOld,
        const std::string& sNew)
{
    std::string::size_type nMatch = 0;
    while ((nMatch = sTarget.find( sOld, nMatch )) != std::string::npos)
    {
        sTarget.replace( nMatch, sOld.length(), sNew );
        nMatch += sNew.length();
    }
}


std::vector<std::string> 
split(const std::string& str, 
      const std::string& delim)
{
    std::vector<std::string> vsRet;
    
//  boost::iter_split( vsRet, str, 
//                     boost::first_finder( delim, boost::is_iequal() ) );
    size_t offset = 0;
    size_t delimIndex = 0;
    delimIndex = str.find(delim, offset);
    while (delimIndex != std::string::npos) {
        vsRet.push_back(str.substr(offset, delimIndex - offset));
        offset += delimIndex - offset + delim.length();
        delimIndex = str.find(delim, offset);
    }
    vsRet.push_back(str.substr(offset));

    return vsRet;
}


std::vector<std::string>
splitAndTrim( const std::string& str, 
              const std::string& delim )
{
    std::vector<std::string> vsRet = split( str, delim );
    std::transform( vsRet.begin(), vsRet.end(), vsRet.begin(), trim );
    return vsRet;
}

   
std::string
toUpper( const std::string& sIn )
{
    std::string sOut = sIn;
    transform( sOut.begin(), sOut.end(), sOut.begin(), toupper );
    return sOut;
}


std::string
toLower( const std::string& sIn )
{
    std::string sOut = sIn;
    transform( sOut.begin(), sOut.end(), sOut.begin(), tolower );
    return sOut;
}


bool isEqualNoCase( const std::string& s1, const std::string& s2)
{
#ifdef WIN32
    return _stricmp( s1.c_str(), s2.c_str() ) == 0;
#else
    return strcasecmp( s1.c_str(), s2.c_str() ) == 0;
#endif
}


// Adaptable functor that tests a char for space-ness.
// See Meyers "Effective STL", Item 40.
struct IsSpace : public std::unary_function<int, bool>
{
    bool operator() ( int ch ) const
    {
        return isspace( ch ) != 0;
    }
};
        

///////////////////////////////////////////////////////////////////////////////
/// Trims whitespace from the beginning and end of a string.
/// @param  sIn The string to trim.
/// @return std::string  A copy of the input string with leading and
///                      trailing whitespace removed.
/// @author dnewton@musicmatch.com @date 07-15-2003
///////////////////////////////////////////////////////////////////////////////
std::string
trim( const std::string& sIn )
{
    return trimLeft( trimRight( sIn ) );
} 


///////////////////////////////////////////////////////////////////////////////
/// Trims whitespace from the beginning of a string.
/// @param  sIn The string to trim.
/// @return std::string  A copy of the input string with leading
///                      whitespace removed.
/// @author dnewton@musicmatch.com @date 07-15-2003
///////////////////////////////////////////////////////////////////////////////
std::string
trimLeft( const std::string& sIn )
{
    std::string sRet( sIn );
    sRet.erase( sRet.begin(), find_if( sRet.begin(), sRet.end(),
                                       std::not1( IsSpace() ) ) );
    return sRet;
}


///////////////////////////////////////////////////////////////////////////////
/// Trims whitespace from the end of a string.
/// @param  sIn The string to trim.
/// @return std::string  A copy of the input string with trailing whitespace removed.
/// <b>Notes:</b>
/// - See Scott Meyers' Effective STL, Item 28 on reverse iterators.  This
/// case is slightly different since we want to start erasing elements AFTER
/// the first whitespace character, so we use rit.base() instead of (++rit).base().
/// @author dnewton@musicmatch.com @date 07-15-2003
///////////////////////////////////////////////////////////////////////////////
std::string
trimRight( const std::string& sIn )
{
    std::string sRet( sIn );
    sRet.erase( (find_if( sRet.rbegin(), sRet.rend(),
                          std::not1( IsSpace() ) )).base(),
                sRet.end() );
    return( sRet );
}


bool
matchesWildcard(const std::string& s,
                const std::string& pattern)
{
    // convert wildcard pattern to a regex expression
    // * becomes .*, any regex special char is protected,
    // all other chars copied.  regular expression is
    // fully anchored and entire string must match.
    bool rval = false;
    std::string reChars("[{()\\*+?|^$.");
    std::string rePattern("^");
    for (size_t i = 0; i < pattern.length(); i++) {
        if (pattern[i] == '*') {
            rePattern.append(".*");
        } else if (reChars.find(pattern[i]) != std::string::npos) {
            rePattern.append("\\");
            rePattern.append(1, pattern[i]);
        } else {
            rePattern.append(1, pattern[i]);
        }
    }
    rePattern.append("$");
    std::tr1::regex rx(rePattern);
    std::tr1::smatch mr;
    if (std::tr1::regex_match(s, mr, rx)) {
        std::string matched = mr.str();
        rval = matched.compare(s) == 0;
    }
    return rval;
}


std::string 
quoteJsonString(const std::string& str)
{
    const char* s = str.c_str();
    if (std::strpbrk(s, "\"\\\b\f\n\r\t") == NULL) {
        return std::string("\"") + s + "\"";
    }
    
    unsigned maxsize = strlen(s)*2 + 3;
    std::string result;
    result.reserve(maxsize);
    result += "\"";
    for (const char* c=s; *c != 0; ++c){
        switch (*c) {
            case '\"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result += *c;
            }
        }
    result += "\"";
    return result;
}


bool
loadFromFile(const bp::file::Path& path, std::string& sOut)
{
    if (!boost::filesystem::is_regular(path)) {
        return false;
    }

    std::ifstream ifs;
    if (!bp::file::openReadableStream(ifs, path, std::ios::binary)) {
        return false;
    }

    sOut.clear();

    // See Meyers "Effective STL", Item 29.
    copy(std::istreambuf_iterator<char>(ifs),
         std::istreambuf_iterator<char>(),
         back_inserter(sOut));

    return true;
}

bool
storeToFile(const bp::file::Path& path, const std::string& sIn)
{
    std::ofstream ofs;
    if (!bp::file::openWritableStream(ofs, path, std::ios::trunc | std::ios::binary)) {
        return false;
    }
    ofs << sIn;
    ofs.close();    

    return (ofs.good() && !ofs.fail());
}

} // namespace string
} // namespace bp
