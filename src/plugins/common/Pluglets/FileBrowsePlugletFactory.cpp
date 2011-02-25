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

#include "FileBrowsePlugletFactory.h"
#include "FileBrowsePluglet.h"

// ------------ version 1

BPArgumentDefinition s_browseArguments[] = {
    {
        (BPString) "recurse",
        (BPString) "If true and a folder is selected, the folder's recursive "
        "contents will be returned.  If false, the folder itself "
        "will be returned.  Default is true.",
        BPTBoolean,
        false
    },
    {
        (BPString) "mimeTypes",
        (BPString) "A list of mimetypes to filter against.  Only items which "
        "match one of these mimetypes will be accepted.  Default is "
        "empty (all items will be accepted)",
        BPTList,
        false
    },
    {
        (BPString) "includeGestureInfo",
        (BPString) "Should selection gesture information be included in the argument "
        "to the callback?  Default is false.  If false, the argument "
        "is an array of opaque file handles.  If true, the argument is a map "
        "containing keys 'actualSelection' and 'files'.  The 'actualSelection' "
		"value is a list of opaque file handles representing what was actually "
		"selected by the UI gesture.  The 'files' value is an array of maps, "
		"each entry containing keys 'handle' (value is an opaque file handle) "
		"and 'parent' (value is handle id of element in 'actualSelection' list "
		"which resulted in this file being included).",
        BPTBoolean,
        false
    },
    {
        (BPString) "limit",
        (BPString) "Maximum number of items which will be included. "
        "Default is 1000",
        BPTInteger,
        false
    }
};

BPFunctionDefinition s_browseFunctions[] = {
    {
        (BPString) "OpenBrowseDialog",
        (BPString) "Present the user with a native browse dialog.  Return value is a "
        "list of filehandles for the selected items.  On OSX and Windows XP, "
        "multiple files and folders may be selected.  On Windows Vista and "
        "Windows 7, multiple files or a single folder may be selected.",
        sizeof(s_browseArguments)/sizeof(s_browseArguments[0]),
        s_browseArguments
    }
};

// a description of this service.
static BPServiceDefinition s_fileBrowsePlugletDef = {
    (BPString) "FileBrowse",
    1, 0, 1,
    (BPString) "Present the user with a file browse dialog.",
    sizeof(s_browseFunctions)/sizeof(s_browseFunctions[0]),
    s_browseFunctions
};

// ------------ version 2

BPFunctionDefinition s_browseFunctions2[] = {
    {
        (BPString) "OpenBrowseDialog",
        (BPString) "Present the user with a native browse dialog.  On OSX and Windows XP, "
		"multiple files and folders may be selected.  On Windows Vista and "
        "Windows 7, multiple files or a single folder may be selected.  "
        "Return value has key \"files\" which contains a list of filehandles "
        "for the selected items.",
        0, NULL
    }
};

// a description of this service.
static BPServiceDefinition s_fileBrowsePlugletDef2 = {
    (BPString) "FileBrowse",
    2, 0, 0,
    (BPString) "Present the user with a file browse dialog.",
    sizeof(s_browseFunctions2)/sizeof(s_browseFunctions2[0]),
    s_browseFunctions2
};

// ------------ version 3

BPArgumentDefinition s_saveArguments[] =
{
    {
        (BPString) "name",
        (BPString) "The default filename.",
        BPTString,
        false
    }
};

BPFunctionDefinition s_browseFunctions3[] = {
    {
        (BPString) "selectFiles",
        (BPString) "Present the user with a native browse dialog.  On OSX and Windows XP, "
		"multiple files and folders may be selected.  On Windows Vista and "
        "Windows 7, multiple files or a single folder may be selected.  "
        "Return value has key \"files\" which contains a list of filehandles "
        "for the selected items.",
        0, NULL
    },
    {
        (BPString) "saveAs",
        (BPString) "Present the user with a native save dialog.  Return value has "
        "key \"file\" which contains a filehandle for the filename "
        "to save to.",
        sizeof(s_saveArguments)/sizeof(s_saveArguments[0]),
        s_saveArguments
    },
};

// a description of this service.
static BPServiceDefinition s_fileBrowsePlugletDef3 = {
    (BPString) "FileBrowse",
    3, 0, 0,
    (BPString) "Present the user with a file browse dialog.",
    sizeof(s_browseFunctions3)/sizeof(s_browseFunctions3[0]),
    s_browseFunctions3
};


FileBrowsePlugletFactory::FileBrowsePlugletFactory()
{
    m_descriptions.push_back(bp::service::Description());
    m_descriptions.back().fromBPServiceDefinition(&s_fileBrowsePlugletDef);
    m_descriptions.push_back(bp::service::Description());
    m_descriptions.back().fromBPServiceDefinition(&s_fileBrowsePlugletDef2);
    m_descriptions.push_back(bp::service::Description());
    m_descriptions.back().fromBPServiceDefinition(&s_fileBrowsePlugletDef3);
}


std::list<Pluglet*>
FileBrowsePlugletFactory::createPluglets(BPPlugin* plugin)
{
    std::list<Pluglet*> rval;
    std::list<bp::service::Description>::const_iterator it;
    for (it = m_descriptions.begin(); it != m_descriptions.end(); ++it) {
        rval.push_back(new FileBrowsePluglet(plugin, *it));
    }
    return rval;
}
