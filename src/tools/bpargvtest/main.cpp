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
 * bpargvtest - a simple test program to validate non-english arguments
 *              are properly received.
 *
 * Author: Lloyd Hilaiel
 * (c) Yahoo! Inc 2009
 */

#include <iostream>
#include "BPUtils/bpconvert.h"
#include "platform_utils/ARGVConverter.h"


int
main(int argc, const char ** argv)
{
    APT::ARGVConverter conv;
    conv.convert(argc, argv);
    
    std::cout << "This program received " << argc << " arguments:"
              <<  std::endl;

    for (int i = 0; i < argc; i++) {
        std::cout << i << ": '" << argv[i] << "'" << std::endl;
    }

    return 0;
}
