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

#ifndef __INSTALLERSKINVERBOSE__
#define __INSTALLERSKINVERBOSE__

#include "InstallerSkinMinimal.h"

#include <string>

/**
 * A non-graphical installer skin that just prints progress with percentages
 */
class InstallerSkinVerbose : public InstallerSkinMinimal
{
  public:
    InstallerSkinVerbose();
    virtual ~InstallerSkinVerbose();

    virtual void startUp(unsigned int width,
                         unsigned int height,
                         std::string title);
    
    virtual void debugMessage(const std::string& sMsg);

    virtual void allDone();

    virtual void progress(int pct);
    
};

#endif
