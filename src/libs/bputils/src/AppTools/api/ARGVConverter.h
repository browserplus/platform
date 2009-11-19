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
 * Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/**
 * ARGVConverter - an abstraction which handles conversion of non-english
 * command line arguments from utf16 to utf8, essentially a noop on unix
 * systems.
 *
 * Created by Lloyd Hilaiel on Tues Mar 5 2009
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#ifndef __APTARGVCONVERTER_H__
#define __APTARGVCONVERTER_H__

#include <string>
#include <list>

namespace APT
{
    class ARGVConverter 
    {
    public:
        ARGVConverter();
        ~ARGVConverter();        

        void convert(int & argc, const char ** &argv);
    private:
        std::list<std::string> m_utf8Args;
        const char ** m_argv;
    };
};

#endif 
