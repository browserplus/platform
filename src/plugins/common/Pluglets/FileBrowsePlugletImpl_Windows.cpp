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

/*
 *  FileBrowsePlugletImpl_Windows.cpp
 *
 *  Created by David Grigsby on 11/20/2007.
 *  Modified to support multiple selects
 *  and mimetype filtering by Gordon Durand on 07/10/08.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 */

#include "FileBrowsePluglet.h"

#include <vector>
#include <string>

#include "bppluginutil.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bplocalization.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpurl.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/OS.h"
#include "CommonErrors.h"
#include "FileBrowseDialogs_Windows.h"


using namespace std;
using namespace bp::localization;
namespace bpf = bp::file;


static wstring getBrowseTitle( const std::string& sUrl,
                               const std::string& sLocale )
{
    std::string sTitle;
    getLocalizedString( FileBrowsePluglet::kSelectFilesFoldersKey, sLocale,
                        sTitle );

    // For security, append the host name.
    bp::url::Url url;
    if (url.parse( sUrl ))
    {
        sTitle += " (";
        sTitle += url.friendlyHostPortString();
        sTitle += ")";
    }

    return bp::strutil::utf8ToWide( sTitle );
}


static bool isXP()
{
    return bp::os::PlatformVersion().compare("6.") < 0;
}


static bool runOsSpecificFileOpenDialog(const FileOpenDialogParms& parms,
                                        vector<bpf::Path>& vPaths)
{
    return isXP() ? runFileOpenDialogXP(parms, vPaths)
                  : runFileOpenDialog(parms, vPaths);
}



////////////////////////////
// FileBrowsePluglet impl

void FileBrowsePluglet::v1Browse(unsigned int tid,
                                 const bp::Object* arguments,
                                 plugletExecutionSuccessCB successCB,
                                 plugletExecutionFailureCB failureCB,
                                 void* callbackArgument)
{
    bool recurse = true;
    std::set<std::string> mimetypes;
    bool includeGestureInfo = false;
    unsigned int limit = 1000;
    wstring filterName;
    wstring filterPattern;
    
    if (!arguments) {
        BPLOG_WARN_STRM("execute called will NULL arguments");
        failureCB(callbackArgument, tid, pluginerrors::InvalidParameters, NULL);
        return;
    }

    if (arguments->has("recurse", BPTBoolean)) {
        recurse = ((bp::Bool*) arguments->get("recurse"))->value();
    }

    if (arguments->has("mimeTypes", BPTList)) {
        const bp::List* l = (const bp::List*) arguments->get("mimeTypes");
        for (unsigned int i = 0; l && (i < l->size()); i++) {
            const bp::String* s = dynamic_cast<const bp::String*>(l->value(i));
            if (s) {
                mimetypes.insert(s->value());
            }
        }
    } 

    if (arguments->has("includeGestureInfo", BPTBoolean)) {
        includeGestureInfo = ((bp::Bool*) arguments->get("includeGestureInfo"))->value();
    }

    if (arguments->has("limit", BPTInteger)) {
        limit = (unsigned int)(((bp::Integer*) arguments->get("limit"))->value());
    }

    // Get extensions for mimetypes and set filter from them
    if (mimetypes.empty()) {
        string lstr;
        (void) getLocalizedString(FileBrowsePluglet::kAllFilesFoldersKey,
                                  m_locale, lstr);
        filterName = bp::strutil::utf8ToWide(lstr);
        filterPattern.append(L"*.*");
    } else {
        set<string> extensions;
        set<string>::const_iterator iter;
        for (iter = mimetypes.begin(); iter != mimetypes.end(); ++iter) {
            filterName.append(bp::strutil::utf8ToWide(*iter));
            filterName.append(L", ");
            vector<string> e = bpf::extensionsFromMimeType(*iter);
            for (unsigned int i  = 0; i < e.size(); ++i) {
                const string& s = e[i];
                extensions.insert(s);
            }
        }
        filterName.erase(filterName.length()-2);  // nuke trailing ', '
        if (!extensions.empty()) {
            for (iter = extensions.begin(); iter != extensions.end(); ++iter) {
                filterPattern.append(L"*.");
                filterPattern.append(bp::strutil::utf8ToWide(*iter));
                filterPattern.append(1, L';');
            }
            filterPattern.erase(filterPattern.length()-1);  // nuke trailing ;
        } else {
            filterPattern.append(L"*.*");
        }
    }

    // Setup dialog title.
    string currUrl;
    // TODO: handle failure of this call, also IDNA on FF?
    m_plugin->getCurrentURL(currUrl);
    wstring title = getBrowseTitle(currUrl, m_locale);
    
    ///
    // Run the dialog.
    bool chaseLinks = true;
    FileOpenDialogParms parms = { (HWND)m_plugin->getWindow(), m_locale, title,
                                  filterName, filterPattern, chaseLinks };
    vector<bpf::Path> vPaths;
    if (!runOsSpecificFileOpenDialog(parms, vPaths))
    {
        failureCB(callbackArgument, tid,
                  "FileBrowse.error", "FileOpenDialog error");
        return;
    }

    // version 1 applies filtering, recurses, etc
    unsigned int flags = 0;
    if (recurse) flags |= bp::pluginutil::kRecurse;
    if (includeGestureInfo) flags |= bp::pluginutil::kIncludeGestureInfo;
    bp::Object* pObj = bp::pluginutil::applyFilters(vPaths, mimetypes, flags, limit);
    successCB(callbackArgument, tid, pObj);
    delete pObj;
}

          
void FileBrowsePluglet::browse(unsigned int tid,
                               plugletExecutionSuccessCB successCB,
                               plugletExecutionFailureCB failureCB,
                               void* callbackArgument)
{
    // Setup dialog title.
    string currUrl;
    // TODO: handle failure of this call, also IDNA on FF?
    m_plugin->getCurrentURL(currUrl);
    wstring title = getBrowseTitle(currUrl, m_locale);
    
    ///
    // Run the dialog.
    bool chaseLinks = false;
    FileOpenDialogParms parms = { (HWND)m_plugin->getWindow(), m_locale, title,
                                  L"", L"", chaseLinks };
    vector<bpf::Path> vPaths;
    if (!runOsSpecificFileOpenDialog(parms, vPaths))
    {
        failureCB(callbackArgument, tid,
                  "FileBrowse.error", "FileOpenDialog error");
        return;
    }
    
    // version 2 and above just return what was selected
    bp::Map* m = new bp::Map;
    bp::List* l = new bp::List;
    vector<bpf::Path>::const_iterator it;
    for (it = vPaths.begin(); it != vPaths.end(); ++it) {
        bpf::Path path(*it);
        l->append(new bp::Path(path));
    }
    m->add("files", l);
    bp::Object* pObj = m;
    successCB(callbackArgument, tid, pObj);
    delete pObj;
}


void FileBrowsePluglet::save(unsigned int tid,
                             const bp::Object* arguments,
                             plugletExecutionSuccessCB successCB,
                             plugletExecutionFailureCB failureCB,
                             void* callbackArgument)
{
    // TODO: title.
    wstring title;

    wstring wsInitialName;
    if (arguments && arguments->has("name", BPTString)) {
        string initialName = ((bp::String*) arguments->get("name"))->value();
        wsInitialName = bp::strutil::utf8ToWide( initialName );
    }
    
    FileSaveDialogParms parms = { (HWND)m_plugin->getWindow(), m_locale, title,
                                   wsInitialName };
    bp::file::Path path;

    if (!runFileSaveDialog(parms, path)) {
        failureCB(callbackArgument, tid,
                  "FileBrowse.error", "FileSaveDialog error.");
        return;
    }

    if (path.empty()) {
        failureCB(callbackArgument, tid, "FileBrowse.userCanceled",
                  "user canceled browse");
        return;
    }
    
    bp::Object* pObj = new bp::WritablePath(path);
    successCB(callbackArgument, tid, pObj);
    delete pObj;
}


