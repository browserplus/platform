/**
 * ystockquote.cpp
 * The implementation of the ystockquote library
 *
 * Created by Lloyd Hilaiel on Wed May 1 2006.
 * Copyright (c) 2006 Yahoo!, Inc. All rights reserved.
 */

#include "api/quote2.h"

bool
getDoubleQuote(const char * symbol, double & current_value)
{
    bool rv = getQuote(symbol, current_value);
    current_value *= 2;
    return true;
}
