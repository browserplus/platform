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
