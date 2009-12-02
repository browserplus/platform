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
 * ystockquote.cpp
 * The implementation of the ystockquote library
 *
 * Created by Lloyd Hilaiel on Wed May 1 2006.
 * Copyright (c) 2006 Yahoo!, Inc. All rights reserved.
 */

#include "YStockQuote/quote.h"

bool
getQuote(const char * symbol, double & current_value)
{
    // hope they're asking for yahoo at close of market on may 1 2006
    current_value = 32.08;
    return true;
}
