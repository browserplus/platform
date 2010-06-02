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

#include "api/bplzma.h"

bp::lzma::IO::IO() : m_inputStream(NULL), m_outputStream(NULL)
{
}

bp::lzma::IO::~IO()
{
}

void
bp::lzma::IO::setInputStream(std::istream & inputStream)
{
    m_inputStream = &inputStream;
}

void
bp::lzma::IO::setOutputStream(std::ostream & outputStream)
{
    m_outputStream = &outputStream;
}

size_t
bp::lzma::IO::outputCallback(void *ctx, const void *buf, size_t size)
{
    std::ostream * os = ((bp::lzma::IO *) ctx)->m_outputStream;

    if (os == NULL || os->eof() || os->fail())
    {
        return 0;
    }

    unsigned int pos = os->tellp();
    if (pos == (unsigned int) -1) pos = 0;
	os->write((const char *) buf, size);
    return ((unsigned int) os->tellp() - pos);
}

int
bp::lzma::IO::inputCallback(void *ctx, void *buf, size_t * size)
{
    std::istream * is = ((bp::lzma::IO *) ctx)->m_inputStream;

    if (is == NULL) return 1;

    if (is->eof()) {
        *size = 0;
    } else {
        is->read((char *) buf, *size);
        *size = is->gcount();
    }

    return 0;
}

