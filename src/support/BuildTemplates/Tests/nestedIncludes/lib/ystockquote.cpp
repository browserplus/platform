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
