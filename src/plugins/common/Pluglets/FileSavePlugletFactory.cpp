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

#include "FileSavePlugletFactory.h"
#include "FileSavePluglet.h"

BPArgumentDefinition s_saveArguments[] =
{
    {
        "name",
        "The default filename.",
        BPTString,
        false
    }
};

BPFunctionDefinition s_saveFunctions[] = {
    {
        "OpenSaveDialog",
        "Present the user with a native save dialog.  Return value has "
        "key \"file\" which contains a filehandle for the filename "
        "to save to.",
        sizeof(s_saveArguments)/sizeof(s_saveArguments[0]),
        s_saveArguments
    }
};

// a description of this service.
static BPServiceDefinition s_fileSavePlugletDef = {
    "FileSave",
    1, 0, 0,
    "Present the user with a file browse dialog.",
    sizeof(s_saveFunctions)/sizeof(s_saveFunctions[0]),
    s_saveFunctions
};


FileSavePlugletFactory::FileSavePlugletFactory()
{
    m_descriptions.push_back(bp::service::Description());
    m_descriptions.back().fromBPServiceDefinition(&s_fileSavePlugletDef);
}


std::list<Pluglet*>
FileSavePlugletFactory::createPluglets(BPPlugin* plugin)
{
    std::list<Pluglet*> rval;
    std::list<bp::service::Description>::const_iterator it;
    for (it = m_descriptions.begin(); it != m_descriptions.end(); ++it) {
        rval.push_back(new FileSavePluglet(plugin, *it));
    }
    return rval;
}
