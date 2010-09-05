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
 * InactiveServicesServiceFactory
 *
 * A factory implementation for the "core" service
 */

#include "InactiveServicesServiceFactory.h"
#include "InactiveServicesService.h"
#include "platform_utils/bplocalization.h"

using namespace std;
using namespace std::tr1;


InactiveServicesServiceFactory::InactiveServicesServiceFactory()
{
}

InactiveServicesServiceFactory::~InactiveServicesServiceFactory()
{
}

shared_ptr<ServiceInstance>
InactiveServicesServiceFactory::instantiateInstance(
    weak_ptr<ServiceExecutionContext> context)
{
    return shared_ptr<InactiveServicesService>(
        new InactiveServicesService(context));
}

bp::service::BuiltInSummary
InactiveServicesServiceFactory::summary()
{
    std::map<std::string, std::string> titles =
        bp::localization::getLocalizations("inactiveServicesTitle");
    std::map<std::string, std::string> summaries =
        bp::localization::getLocalizations("inactiveServicesSummary");
    
    std::map<std::string, std::string>::iterator i, i2;

    std::map<std::string, std::pair<std::string, std::string> > l10n;
    for (i = titles.begin(); i != titles.end(); i++)  {
        std::string locale = i->first;
        std::string title = i->second;
        std::string summary;
        
        if (summaries.end() != (i2 = summaries.find(locale))) {
            summary = i2->second;
        }

        l10n[locale] = std::pair<std::string, std::string>(title, summary);
    }

    bp::service::BuiltInSummary s("InactiveServices", "1.0.0");
    s.setLocalizations(l10n);

    return s;
}
