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
 * Portions created by Yahoo! are Copyright (c) 2010 Yahoo! Inc.
 * All rights reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/**
 * getquote.cpp
 * A binary we ship with our library that uses the library to get a
 * quote.  
 *
 * Created by Lloyd Hilaiel on Wed May 1 2006.
 * Copyright (c) 2006 Yahoo!, Inc. All rights reserved.
 */

#include <iostream>
#include "YStockQuote/YStockQuote.h"

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
