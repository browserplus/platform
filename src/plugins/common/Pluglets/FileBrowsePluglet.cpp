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

#include "FileBrowsePluglet.h"

const char* FileBrowsePluglet::kSelectKey = "FileBrowsePluglet::kSelectKey";
const char* FileBrowsePluglet::kFileFolderNameKey = "FileBrowsePluglet::kFileFolderNameKey";
const char* FileBrowsePluglet::kFileNameKey = "FileBrowsePluglet::kFileNameKey";
const char* FileBrowsePluglet::kAllFilesFoldersKey = "FileBrowsePluglet::kAllFilesFoldersKey";
const char* FileBrowsePluglet::kSelectFilesFoldersKey = "FileBrowsePluglet::kSelectFilesFoldersKey";
const char* FileBrowsePluglet::kSelectFilesKey = "FileBrowsePluglet::kSelectFilesKey";
const char* FileBrowsePluglet::kSelectFolderKey = "FileBrowsePluglet::kSelectFolderKey";

BPArgumentDefinition s_browseArguments[] = {
    {
        "recurse",
        "If true and a folder is selected, the folder's recursive "
        "contents will be returned.  If false, the folder itself "
        "will be returned.  Default is true.",
        BPTBoolean,
        false
    },
    {
        "mimeTypes",
        "A list of mimetypes to filter against.  Only items which "
        "match one of these mimetypes will be accepted.  Default is "
        "empty (all items will be accepted)",
        BPTList,
        false
    },
    {
        "includeGestureInfo",
        "Should selection gesture information be included in the argument "
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
        "limit",
        "Maximum number of items which will be included. "
        "Default is 10000",
        BPTInteger,
        false
    }
};

BPFunctionDefinition s_browseFunctions[] = {
    {
        "OpenBrowseDialog",
        "Present the user with a native browse dialog.  On OSX and Windows XP, "
		"multiple files and folders may be selected.  On Windows Vista, multiple "
		"files or a single folder may be selected.",
        sizeof(s_browseArguments)/sizeof(s_browseArguments[0]),
        s_browseArguments
    }
};

// a description of this corelet.
static BPCoreletDefinition s_fileBrowseCoreletDef = {
    "FileBrowse",
    1, 0, 1,
    "Present the user with a file browse dialog.",
    sizeof(s_browseFunctions)/sizeof(s_browseFunctions[0]),
    s_browseFunctions
};

FileBrowsePluglet::FileBrowsePluglet(BPPlugin * plugin)
    : Pluglet(plugin)
{
    m_desc.fromBPCoreletDefinition(&s_fileBrowseCoreletDef);
}

FileBrowsePluglet::~FileBrowsePluglet()
{
}

const bp::service::Description *
FileBrowsePluglet::describe()
{
    return &m_desc;
}
