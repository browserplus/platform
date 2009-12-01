/**
 * getquote.cpp
 * A binary we ship with our library that uses the library to get a
 * quote.  
 *
 * Created by Lloyd Hilaiel on Wed May 1 2006.
 * Copyright (c) 2006 Yahoo!, Inc. All rights reserved.
 */

#include <iostream>
#include "YStockQuote.h"

int
main(int argc, char ** argv)
{
    if (argc != 2) {
        std::cout << argv[0] << ": get a stock quote" << std::endl;
        std::cout << "usage: " << argv[0] << " <symbol>" << std::endl;        
        exit(0);
    }
    
    double result = 0.0;

    if (!getQuote(argv[1], result)) {
        std::cout << "error fetching quote for '" << argv[1] << "'"
                  << std::endl;                
        exit(1);
    } else {
        std::cout << argv[1] << " is at " << result << std::endl;
    }

    return 0;
}
