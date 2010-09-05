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
 *  ServiceInterfaceCache - wrappers around lower level bputils facilities
 *                          to give a high level API for caching serialized
 *                          service descriptions.
 *                          
 * (c) 2008 Yahoo!
 */

#ifndef __SERVICE_INTERFACE_CACHE_H__
#define __SERVICE_INTERFACE_CACHE_H__

#include "BPUtils/bptime.h"
#include "BPUtils/bptypeutil.h"


namespace bp { namespace serviceInterfaceCache {

  /* check if a cached interface exists for the service name/version that
   * is newer than the provided time */ 
  bool isNewerThan(const std::string & name,
                   const std::string & version,
                   const BPTime &t);
  
  /** get a dynamically allocated memory representation of the serialized
   *  interface for a specified service from the cache.  NULL if no entry
   *  exists */
  bp::Object * get(const std::string & name,
                   const std::string & version);

  /** add a cache entry for a specified service from the cache.  false
   *  return upon error */
  bool set(const std::string & name, 
           const std::string & version,
           const bp::Object * obj);
  
  /** purge the cache entry for a specified service. */
  bool purge(const std::string & name, 
             const std::string & version);

} }

#endif
