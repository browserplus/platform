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

/**
 * SemanticVersion
 *
 * A Class for representing, comparing and parsing versions
 */

#include "bpsemanticversion.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#define snprintf _snprintf_s
#endif

bp::SemanticVersion::SemanticVersion()
    : m_major(-1), m_minor(-1), m_micro(-1), m_nano(-1)
{
}


bp::SemanticVersion::~SemanticVersion()
{
}

bp::SemanticVersion&
bp::SemanticVersion::operator=(const bp::SemanticVersion& in)
{
    m_major = in.m_major;
    m_minor = in.m_minor;    
    m_micro = in.m_micro;
    m_nano = in.m_nano;
    return *this;
}


int
bp::SemanticVersion::majorVer() const
{
    return m_major;
}


void
bp::SemanticVersion::setMajor(int major)
{
    m_major = major;
}


int
bp::SemanticVersion::minorVer() const
{
    return m_minor;
}


void
bp::SemanticVersion::setMinor(int minor)
{
    m_minor = minor;
}


int
bp::SemanticVersion::microVer() const
{
    return m_micro;
}


void
bp::SemanticVersion::setMicro(int micro)
{
    m_micro = micro;
}


int
bp::SemanticVersion::nanoVer() const
{
    return m_nano;
}


void
bp::SemanticVersion::setNano(int nano)
{
    m_nano = nano;
}


int
bp::SemanticVersion::compare(const bp::SemanticVersion& other) const
{
    int rv = 0;

    // compare major
    if (m_major >= 0 && other.m_major >= 0) {
        rv = (m_major > other.m_major) ? 1 : ((m_major < other.m_major) ? -1 : 0);
    }

    // compare minor
    if (rv == 0) {
        if (m_minor >= 0 && other.m_minor >= 0) {
            rv = (m_minor > other.m_minor) ? 1 :
                 ((m_minor < other.m_minor) ? -1 : 0);
        }
    }

    // compare micro
     if (rv == 0) {
         if (m_micro >= 0 && other.m_micro >= 0) {
             rv = (m_micro > other.m_micro) ? 1 :
                  ((m_micro < other.m_micro) ? -1 : 0);
         }
     }

     // compare nano
      if (rv == 0) {
          if (m_nano >= 0 && other.m_nano >= 0) {
              rv = (m_nano > other.m_nano) ? 1 :
                   ((m_nano < other.m_nano) ? -1 : 0);
          }
      }

      return rv;
}


bool
bp::SemanticVersion::match(const bp::SemanticVersion& other) const
{
    if (m_major >= 0 && other.m_major >= 0 && m_major != other.m_major) {
        return false;
    }

    if (m_minor >= 0 && other.m_minor >= 0 && m_minor != other.m_minor) {
        return false;
    }

    if (m_micro >= 0 && other.m_micro >= 0 && m_micro != other.m_micro) {
        return false;
    }

    if (m_nano >= 0 && other.m_nano >= 0 && m_nano != other.m_nano) {
        return false;
    }

    return true;
}


bool 
bp::SemanticVersion::withinRange(const bp::SemanticVersion& vmin,
                                 const bp::SemanticVersion& vmax) const
{
    return compare(vmin) >= 0 && compare(vmax) <= 0;
}


#define CV_END 0
#define CV_DOT 1
#define CV_NUMBER 2
#define CV_ERROR 3
static
unsigned int cv_lex(const char* str, int& off)
{
    switch (str[off]) {
    case 0: return CV_END;
    case '.': off++; return CV_DOT;
    case '0': case '1': case '2': case '3': case '4':  
    case '5': case '6': case '7': case '8': case '9':   
        while (str[off] <= '9' && str[off] >= '0') off++;
        return CV_NUMBER;
    }
    return CV_ERROR;
}


bool
bp::SemanticVersion::parse(const char* version)
{
    if (version == NULL || !strlen(version)) return true;
    
    int off, nxt, rv;
    off = nxt = 0;

    // parse through the string

    // major version
    rv = cv_lex(version, nxt);
    if (rv != CV_NUMBER) return false;
    m_major = atoi(version);

    // dot
    rv = cv_lex(version, nxt);
    if (rv != CV_END && rv != CV_DOT) return false;

    if (rv != CV_END) {
        // minor version
        off = nxt;
        rv = cv_lex(version, nxt);
        if (rv != CV_NUMBER) return false;
        m_minor = atoi(version + off);

        // dot
        rv = cv_lex(version, nxt);
        if (rv != CV_END && rv != CV_DOT) return false;

        if (rv != CV_END) {
            // micro version
            off = nxt;
            rv = cv_lex(version, nxt);
            if (rv != CV_NUMBER) return false;
            m_micro = atoi(version + off);
        }

        // dot
        rv = cv_lex(version, nxt);
        if (rv != CV_END && rv != CV_DOT) return false;

        if (rv != CV_END) {
            // nano version
            off = nxt;
            rv = cv_lex(version, nxt);
            if (rv != CV_NUMBER) return false;
            m_nano = atoi(version + off);
        }
    }

    return true;
}


bool
bp::SemanticVersion::parse(const std::string& version)
{
    return parse(version.c_str());
}


std::string
bp::SemanticVersion::asString(void) const
{
    const int bufSize = 50;
    char buf[bufSize];
    std::string str;
    if (m_major >= 0) {
        snprintf(buf, bufSize, "%d", m_major);
        str.append(buf);
        
        if (m_minor >= 0) {
            str.append(".");
            snprintf(buf, bufSize, "%d", m_minor);
            str.append(buf);    

            if (m_micro >= 0) {
                str.append(".");
                snprintf(buf, bufSize, "%d", m_micro);
                str.append(buf);
            }

            if (m_nano >= 0) {
                str.append(".");
                snprintf(buf, bufSize, "%d", m_nano);
                str.append(buf);
            }
        }
    }

    return str;
}


bool
bp::SemanticVersion::isNewerMatch(const SemanticVersion& current,
                                  const SemanticVersion& got,
                                  const SemanticVersion& wantver,
                                  const SemanticVersion& wantminver)
{
    // For 'current' to be a winner it must:
    // 1. be newer than what we've already 'got'
    // 2. match the wantver
    // 3. be greater than the minver

    // (1)
    if (current.compare(got) < 0) return false;

    // (2)
    if (!current.match(wantver)) return false;

    // (3)
    if (current.compare(wantminver) < 0) return false;

    return true;
}

