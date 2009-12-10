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

FileBrowsePluglet::FileBrowsePluglet(BPPlugin * plugin,
                                     const bp::service::Description & desc)
    : Pluglet(plugin, desc)
{
}


FileBrowsePluglet::~FileBrowsePluglet()
{
}
