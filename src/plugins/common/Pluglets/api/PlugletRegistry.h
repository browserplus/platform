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
 * Portions created by Yahoo! are Copyright (c) 2009 Yahoo! Inc.
 * All rights reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/**
 * A per-session pluglet registry
 */

#ifndef __PLUGLETREGISTRY_H__
#define __PLUGLETREGISTRY_H__

#include "Pluglet.h"

#include <list>

/**
 * The pluglet registry manages the set of available pluglets.
 */
class PlugletRegistry
{
  public:
    /** allocate a pluglet registry */
    PlugletRegistry();

    /** destroy a pluglet registry */
    ~PlugletRegistry();

    /** register a new pluglet in the pluglet registry.
     *  After registering a pluglet, the PlugletRegistry assumes
     *  responsibility for the memory.  When the registry is freed,
     *  the pluglet will be release.
     *  
     *  \returns false if an error is encountered (pluglet already
     *           registered with same name 
     */
    bool registerPluglet(Pluglet * pluglet);

    /**
     * Find a pluglet with a matching name, version, and minversion.
     *
     * \returns NULL if no such pluglet exists
     */
    Pluglet * find(std::string name, std::string version,
                   std::string minversion);
    
    /** get a list of all available pluglets */
    std::list<Pluglet *> availablePluglets();

  private:
    std::list<Pluglet *> m_pluglets;
};

#endif
