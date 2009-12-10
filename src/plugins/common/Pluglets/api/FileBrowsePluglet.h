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
 * The interface for the File Browse Pluglet
 */

#ifndef __FILEBROWSEPLUGLET_H__
#define __FILEBROWSEPLUGLET_H__

#include "PluginCommonLib/Pluglet.h"
#include "PluginCommonLib/BPPlugin.h"
#include <list>

class FileBrowsePluglet : public Pluglet
{
  public:
    /** localization keys */
    static const char* kSelectKey;
    static const char* kFileFolderNameKey;
    static const char* kFileNameKey;
    static const char* kAllFilesFoldersKey;
    static const char* kSelectFilesFoldersKey;
    static const char* kSelectFilesKey;
    static const char* kSelectFolderKey;

    FileBrowsePluglet(BPPlugin * plugin,
                      const bp::service::Description& desc);
    virtual ~FileBrowsePluglet();

    /** defined per plugin per platform */
    void execute(unsigned int tid,
                 const char * function,
                 const bp::Object * arguments,
                 bool syncInvocation,
                 plugletExecutionSuccessCB successCB,
                 plugletExecutionFailureCB failureCB,
                 plugletInvokeCallbackCB   callbackCB,
                 void * callbackArgument);
};

#endif
