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
 */

#include "api/bplzma.h"

#include "easylzma/compress.h"

#include <assert.h>

bp::lzma::Compress::Compress() : IO()
{
}

bp::lzma::Compress::~Compress()
{
}

bool
bp::lzma::Compress::run(Format format)
{
    if (m_inputStream == NULL || m_outputStream == NULL) {
        return false;
    }

    int rc;
    elzma_compress_handle hand;

    /* allocate compression handle */
    hand = elzma_compress_alloc();
    assert(hand != NULL);

    /* calculate the uncompressed size */
    m_inputStream->seekg(0, std::ios_base::end);
    long long unsigned int len = m_inputStream->tellg();
    m_inputStream->seekg(0, std::ios_base::beg);    

    elzma_file_format f = (format == LZIP) ? ELZMA_lzip : ELZMA_lzma;

    rc = elzma_compress_config(hand, ELZMA_LC_DEFAULT,
                               ELZMA_LP_DEFAULT, ELZMA_PB_DEFAULT,
                               5, elzma_get_dict_size(len),
                               f, len);

    if (rc != ELZMA_E_OK) {
        elzma_compress_free(&hand);
        return false;
    }    

    /* now run the compression */
    rc = elzma_compress_run(hand, inputCallback, (void *) this,
                            outputCallback, (void *) this,
                            NULL, NULL);

    elzma_compress_free(&hand);    

    return (rc == ELZMA_E_OK);
}
