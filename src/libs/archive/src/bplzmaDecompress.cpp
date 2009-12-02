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

#include "api/bplzma.h"

#include "easylzma/decompress.h"

bp::lzma::Decompress::Decompress() : IO()
{
}

bp::lzma::Decompress::~Decompress()
{
}

bool
bp::lzma::Decompress::run(Format format)
{
    int rc;
    elzma_decompress_handle hand;
    
    hand = elzma_decompress_alloc();

    elzma_file_format f = (format == LZIP) ? ELZMA_lzip : ELZMA_lzma;    

    /* now run the compression */

    rc = elzma_decompress_run(hand, inputCallback, (void *) this,
                              outputCallback, (void *) this, f);

    elzma_decompress_free(&hand);

    return (rc == ELZMA_E_OK);
}
