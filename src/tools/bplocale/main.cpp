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
 * bplocale - a command line utility for testing locale discovery and
 *            fallback
 *
 * Author: Lloyd Hilaiel
 * (c) Yahoo! Inc 2009
 */

#include <iostream>
#include "BPUtils/APTArgParse.h"
#include "platform_utils/bplocalization.h"

// definition of program arguments
static APTArgDefinition s_args[] =
{
    {
        "locale", false, "", false, false, false,
        "Print fallbacks for the given locale rather than the system default"
    }
};

int
main(int argc, const char ** argv)
{
    // process command line arguments
    APTArgParse ap(" <options>\n  given a locale (either provided on the command line or pulled from the system)\n  display the full list of fallbacks");
    int nargs;
    nargs = ap.parse(sizeof(s_args) / sizeof(s_args[0]), s_args, argc, argv);
    if (nargs < 0) {
        std::cerr << ap.error();
        return 1;
    }
    
    std::string locale = ap.argument("locale");

    if (locale.empty()) locale = bp::localization::getUsersLocale();

    std::vector<std::string> lc =
        bp::localization::getLocaleCandidates(locale);

    std::cout << "for locale: " << locale << std::endl;
    for (unsigned int i = 0; i < lc.size(); i++) {
        std::cout << "    "<<i+1<<": " << lc[i] << std::endl;        
    }

    return 0;
}
