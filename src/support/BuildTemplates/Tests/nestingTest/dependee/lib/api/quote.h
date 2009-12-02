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
 * quote.h
 * The public interface for the ystockquote library
 *
 * Created by Lloyd Hilaiel on Wed May 1 2006.
 * Copyright (c) 2006 Yahoo!, Inc. All rights reserved.
 */

#ifndef __QUOTE_H__
#define __QUOTE_H__

/**
 * get a quote for the specified symbol.
 * PARAMS:
 *  symbol - the symbol to look up
 *  current_value - an output parameter which upon success contains the
 *                  current value of the stock
 * RETURNS:
 *  true on success.  false on failure.
 */
bool YStockQuote_API getQuote(const char * symbol, double & current_value);

#endif
