/**
 * quote.h
 * The public interface for the ystockquote library
 *
 * Created by Lloyd Hilaiel on Wed May 1 2006.
 * Copyright (c) 2006 Yahoo!, Inc. All rights reserved.
 */

#ifndef __QUOTE2_H__
#define __QUOTE2_H__

#include "YStockQuote/quote.h"

/**
 * get a quote for the specified symbol.
 * PARAMS:
 *  symbol - the symbol to look up
 *  current_value - an output parameter which upon success contains the
 *                  current value of the stock
 * RETURNS:
 *  true on success.  false on failure.
 */
bool YStockQuote_API getDoubleQuote(const char * symbol,
				    double & current_value);

#endif
