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

/**
 * bplzma.h - an abstraction around public domain licensed LZMA
 *            compression and decompression using the easylzma library.
 *
 * TODO:
 *   * progress callback during compression (maybe decompression too)
 */

#ifndef __BPLZMA_H__
#define __BPLZMA_H__

#include <string>
#include <vector>

#include <iostream>

namespace bp { namespace lzma {

typedef enum {
    LZMA, LZIP
} Format;

/**
 * a common base class that encapsulates reading and writing to std streams
 */
class IO 
{
  public:
    IO();
    ~IO();
    
    void setInputStream(std::istream & inputStream);
    void setOutputStream(std::ostream & outputStream);

  protected:
    std::istream * m_inputStream;
    std::ostream * m_outputStream;

    // callbacks required by easylzma
    static size_t outputCallback(void *ctx, const void *buf, size_t size);
    static int inputCallback(void *ctx, void *buf, size_t * size);
};
        
class Decompress : public IO
{
  public:
    Decompress();
    ~Decompress();    
    
    bool run(Format format = LZIP);
};

class Compress : public IO
{
  public:
    Compress();
    ~Compress();    
    
    bool run(Format format = LZIP);    
};

}; };

#endif
