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
 * CoreletFactory
 *
 * A factory capable of creating corelet instances.
 */

#ifndef __CORELETFACTORY_H__
#define __CORELETFACTORY_H__

#include "BPUtils/ServiceSummary.h"
#include "CoreletExecutionContext.h"
#include "CoreletInstance.h"


class CoreletFactory
{
public:
    CoreletFactory();
    virtual ~CoreletFactory();

    virtual bp::service::BuiltInSummary summary() = 0;

    virtual std::tr1::shared_ptr<CoreletInstance>
        instantiateInstance(std::tr1::weak_ptr<CoreletExecutionContext> context)
        = 0;
};

#endif
