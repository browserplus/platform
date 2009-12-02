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
 *  bpicu.h
 *
 *  BrowserPlus ICU abstractions.
 *  
 *  Created by David Grigsby on 2/19/09.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef _BPICU_H_
#define _BPI1CU_H_

#include <string>
#include "BPUtils/bperrorutil.h"


namespace bp {
namespace i18n {

// Represents errors reported by the ICU package.
// Note: The general approach of the I18n facility is to hide ICU as an
// implementation detail.  In the case of errors however, it is
// valuable to know that an error originated in the ICU package.
class IcuException : public bp::error::Exception
{
public:
    IcuException( const std::string& sDesc ) :
        bp::error::Exception( sDesc ) {}
};

} // i18n
} // bp



///////////////////////////////////////////////////////////////////////////
// Error handling macro
// This is primarily for I18n package internal use.
#define BP_CHECK_ICU( stat )                                            \
{                                                                       \
    if (U_FAILURE( (stat) ))                                            \
    {                                                                   \
        BP_THROW_TYPE( bp::i18n::IcuException, u_errorName( (stat) ));  \
    }                                                                   \
}



#endif
