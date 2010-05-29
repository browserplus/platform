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
 *  Html5DropManager.cpp
 *  BrowserPlusPlugin
 *
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */

#include "Html5DropManager.h"
#include "nputils.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpurl.h"
#include "BPUtils/bpfile.h"

#include "PluginCommonLib/IDropManager.h"
#include "PluginCommonLib/IDropListener.h"

#include "NPAPIVariant.h"

using namespace std;

// a JavaScript function to add HTML5 style handlers to a supplied dom id, which call back into the supplied 
static const char * s_addDropTargetFunc =
"function (platformArg, browserArg, id, cbObj) {\n"
"  if (!cbObj || !cbObj.onEnter || !cbObj.onExit || !cbObj.onDrop) { \n"
"    throw 'error adding target, bogus callback object!';\n"
"  }\n"
"\n"
"  var elem = document.getElementById(id);\n"
"  var platform = platformArg;\n"
"  var browser = browserArg;\n"
"\n"
"  if (elem == null) {\n"
"    throw 'can\\'t find element with id: \\'' + id + '\\'';\n"
"  }\n"
"\n"
"  if (elem.ondragenter || elem.ondragleave || elem.ondrop) {\n"
"    throw 'element already has non-nil value for .ondragenter/.ondragleave/.ondrop -- BrowserPlus cannot attach drop target: \\'' + id + '\\'';\n"
"  }\n"
"\n"
"  var isRealFile = function(f) {\n"
"    var hasFileName = false, hasFileSize = false, mutableMembers = false;\n"
"\n"
"    for (var m in f) {\n"
"      if (m === 'fileName') hasFileName = true;\n"
"      else if (m === 'fileSize') hasFileSize = true;\n"
"    }\n"
"    try {\n"
"      var before = f['fileName'];\n"
"      f['fileName'] = '__IMutedYou__';\n"
"      if (before !== f['fileName']) mutableMembers = true;\n"
"      before = f['fileSize'];\n"
"      f['fileSize'] = '__IMutedYou__';\n"
"      if (before !== f['fileSize']) mutableMembers = true;\n"
"    } catch (e) {\n"
"    }\n"
"    if (typeof(f) !== 'object' ||\n"
"        !f.toString || f.toString.constructor !==  Function ||\n"
"        f.toString() !== '[object File]' ||\n"
"        f.constructor != File ||\n"
"        f.protype != undefined ||\n"
"        !hasFileName ||\n"
"        !hasFileSize ||\n"
"        mutableMembers) {\n"
"      return false;\n"
"    }\n"
"\n"
"    return true;\n"
"  };\n"
"\n"
"  var extractDragItems = function(event) {\n"
"    var dt = event.dataTransfer;\n"
"    var files = dt.files;\n"
"    var uris = [ ];\n"
"    if (platform == 'Windows' && browser == 'Firefox') {\n"
"      for (var i = 0; i < dt.mozItemCount; i++) {\n"
"        uris[i] = dt.mozGetDataAt('text/x-moz-url', i);\n"
"      }\n"
"    } else if (platform == 'OSX' && browser == 'Safari') {\n"
"      var uriText = dt.getData('text/uri-list');\n"
"      if (uriText) uris = uriText.split('\\n');\n"
"    }\n"
"\n"
"    if (uris.length != files.length) {\n"
"      throw 'uri/files size mismatch, aborting drop';\n"
"    }\n"
"\n"
"    var arr = new Array;\n"
"    for (var i = 0; i < files.length; i++) {\n"
"      if (!isRealFile(files[i])) {\n"
"          throw 'bogus user File data found, possible attack detected!';\n"
"      }\n"
"      arr.push([ files[i], uris[i] ]);\n"
"    }\n"
"    return arr;\n"
"  };\n"
"\n"
"  var insideCnt = 0;\n"
"\n"
"  /** Adapted from YUI - get the dimensions of a dom node */\n"
"  function getDimensions(el) {\n"
"    if (!el) { return false; }\n"
"    // has to be part of document to have pageXY\n"
"    if (el.parentNode === null || el.style.display == 'none') {\n"
"      return false;\n"
"    }\n"
"    var pos = [el.offsetLeft, el.offsetTop];\n"
"    var parentNode = el.offsetParent;\n"
"    if (parentNode != el) {\n"
"      while (parentNode) {\n"
"        pos[0] += parentNode.offsetLeft;\n"
"        pos[1] += parentNode.offsetTop;\n"
"        parentNode = parentNode.offsetParent;\n"
"      }\n"
"    }\n"
"    parentNode = el.parentNode;\n"
"    \n"
"    // account for any scrolled ancestors\n"
"    while (parentNode && parentNode.tagName.toUpperCase() != 'HTML') {\n"
"      pos[0] -= parentNode.scrollLeft;\n"
"      pos[1] -= parentNode.scrollTop;\n"
"      parentNode = parentNode.parentNode;\n"
"    }\n"
"  \n"
"    return {\n"
"      y: parseInt(pos[1]),\n"
"      x: parseInt(pos[0]),\n"
"      y1: (parseInt(pos[1]) + parseInt(el.offsetHeight)),\n"
"      x1: (parseInt(pos[0]) + parseInt(el.offsetWidth))\n"
"    };\n"
"  }\n"
"  \n"
"  function eventInsideTarget(target, event) {\n"
"    var cs = getDimensions(target);\n"
"    return (cs.x <= event.clientX && event.clientX <= cs.x1 && cs.y <= event.clientY && event.clientY <= cs.y1);\n"
"  } \n"
"  \n"
"  elem.ondragenter = function(event) {\n"
"    event.stopPropagation(); event.preventDefault(); \n"
"    if (eventInsideTarget(event.currentTarget, event)) {\n"
"      if (0 == (insideCnt++)) {\n"
"        cbObj.onEnter(extractDragItems(event));\n"
"      }\n"
"    }\n"
"  };\n"
"\n"
"  elem.ondragleave =  function(event) {\n"
"    event.stopPropagation(); event.preventDefault(); \n"
"    if (insideCnt > 0) {\n"
"      insideCnt--;\n"
"      if (!insideCnt || !eventInsideTarget(event.currentTarget, event)) {\n"
"        insideCnt = 0;\n"
"        cbObj.onExit();\n"
"      }\n"
"    }\n"
"  };\n"
"  \n"
"  elem.ondragover = function(event) {\n"
"    event.stopPropagation(); event.preventDefault(); \n"
"    var hit = eventInsideTarget(event.currentTarget, event);\n"
"    if (hit && insideCnt == 0) {\n"
"       insideCnt++; \n"
"       cbObj.onEnter();\n"
"    } else if (!hit && insideCnt > 0) {\n"
"      if (0 == --insideCnt) cbObj.onExit();\n"
"    }\n"
"  };\n"
"  \n"
"  elem.ondrop = function(event) {\n"
"    event.stopPropagation(); event.preventDefault(); \n"
"    insideCnt = 0;\n"
"    if (eventInsideTarget(event.currentTarget, event)) {\n"
"      cbObj.onDrop(extractDragItems(event));\n"
"    }\n"
"  };\n"
"  \n"
"  return null;\n"
"}\n";

static const char * s_removeDropTargetFunc =
"  function(id) {\n"
"  var elem = document.getElementById(id);\n"
"  elem.ondrop = elem.ondragover = elem.ondragleave = elem.ondragenter = null;\n"
"  }\n";

IDropManager*
Html5DropManager::allocate(NPP instance,
                           NPWindow* window,
                           IDropListener* dl,
                           const string& platform,
                           const string& browser)
{
    return new Html5DropManager(instance, window, dl, platform, browser);
}

Html5DropManager::Html5DropManager(NPP npp,
                                   NPWindow* window,
                                   IDropListener* dl,
                                   const string& platform,
                                   const string& browser)
    : m_npp(npp), m_addDropTargetFunc(NULL), m_removeDropTargetFunc(NULL),
      m_listener(dl), m_platform(platform), m_browser(browser)
{
    BPLOG_DEBUG_STRM("new Html5DropManager");

    m_addDropTargetFunc = npu::createJSFunc(npp, s_addDropTargetFunc);
    if (!m_addDropTargetFunc) BPLOG_FATAL("Couldn't compile addDropTargetFunc function!");

    m_removeDropTargetFunc = npu::createJSFunc(npp, s_removeDropTargetFunc);
    if (!m_removeDropTargetFunc) BPLOG_FATAL("Couldn't compile removeDropTargetFunc function!");
}

Html5DropManager::~Html5DropManager()
{
    BPLOG_DEBUG_STRM("delete Html5DropManager");
    if (m_addDropTargetFunc) npu::releaseObject(m_addDropTargetFunc);
    if (m_removeDropTargetFunc) npu::releaseObject(m_removeDropTargetFunc);
    m_addDropTargetFunc = NULL;
}


bool
Html5DropManager::addTarget(const std::string& element,
                            const std::set<std::string>& mimeTypes,
                            bool includeGestureInfo,
                            unsigned int limit)
{
    // empty id is meaningless
    if (element.empty()) return false;

    // check if this is already registered on m_targets
    if (m_targets.find(element) != m_targets.end()) return false;

    size_t argNum = 0;
    NPVariant args[4];
    STRINGZ_TO_NPVARIANT(m_platform.c_str(), args[argNum]);
    argNum++;
    STRINGZ_TO_NPVARIANT(m_browser.c_str(), args[argNum]);
    argNum++;

    // Ask browser for bounds of element associated with drop target
    STRINGZ_TO_NPVARIANT(element.c_str(), args[argNum]);
    argNum++;

    NPVariant result;
    result.type = NPVariantType_Void;

    // now allocate the context for this drop target
    DropTargetContext * ctx = new DropTargetContext(m_npp, element,
                                                    mimeTypes,
                                                    includeGestureInfo,
                                                    limit, this);

    args[argNum].type = NPVariantType_Object;
    args[argNum].value.objectValue = ctx->callbackObject();
    argNum++;
        
    if (npu::callFunction(m_npp, m_addDropTargetFunc, args, argNum, &result))
    {
        // if the function is successfully invoked, then we'll add this target
        m_targets[element] = ctx;
    }
    else
    {
        BPLOG_WARN_STRM("couldn't add target for id: " << element);        
        delete ctx;
    }

    return true;
}


bool
Html5DropManager::addTarget(const std::string& element,
                            const std::string& version)
{
    // empty id is meaningless
    if (element.empty()) return false;

    // check if this is already registered on m_targets
    if (m_targets.find(element) != m_targets.end()) return false;

    size_t argNum = 0;
    NPVariant args[4];
    STRINGZ_TO_NPVARIANT(m_platform.c_str(), args[argNum]);
    argNum++;
    STRINGZ_TO_NPVARIANT(m_browser.c_str(), args[argNum]);
    argNum++;

    // Ask browser for bounds of element associated with drop target
    STRINGZ_TO_NPVARIANT(element.c_str(), args[argNum]);
    argNum++;

    NPVariant result;
    result.type = NPVariantType_Void;

    // now allocate the context for this drop target
    DropTargetContext * ctx = new DropTargetContext(m_npp, element,
                                                    version, this);

    args[argNum].type = NPVariantType_Object;
    args[argNum].value.objectValue = ctx->callbackObject();
    argNum++;
        
    if (npu::callFunction(m_npp, m_addDropTargetFunc, args, argNum, &result))
    {
        // if the function is successfully invoked, then we'll add this target
        m_targets[element] = ctx;
    }
    else
    {
        BPLOG_WARN_STRM("couldn't add target for id: " << element);        
        delete ctx;
    }

    return true;
}


bool
Html5DropManager::removeTarget(const std::string& element)
{
    BPLOG_DEBUG_STRM("removeTarget");
    return true;
}


bool 
Html5DropManager::registerDropListener(IDropListener* listener)
{
    BPLOG_DEBUG_STRM("registerDropListener");
    return true;
}


// ___DELETE ME___
bool
Html5DropManager::enableTarget(const string& name, bool enable)
{
    // TODO: we should remove enable/disable remnants from source.
    //       client code should just remove/re-add.
    return true;
}

static std::string s_onEnter("onEnter");
static std::string s_onExit("onExit");
static std::string s_onDrop("onDrop");

Html5DropManager::DropTargetContext::DropTargetContext(NPP npp,
                                                       const string& name,
                                                       const set<string>& mimetypes,
                                                       bool includeGestureInfo,
                                                       unsigned int limit,
                                                       Html5DropManager * theMan)
    : DropTargetBase(name, mimetypes, includeGestureInfo, limit), m_npp(npp), m_go(NULL), m_theMan(theMan)
{
    m_go = (BPGenericObject *) BPGenericObject::getObject(npp);
    m_go->defineFunction(s_onEnter, this);
    m_go->defineFunction(s_onExit, this);
    m_go->defineFunction(s_onDrop, this);
}

Html5DropManager::DropTargetContext::DropTargetContext(NPP npp,
                                                       const string& name,
                                                       const string& version,
                                                       Html5DropManager * theMan)
    : DropTargetBase(name, version), m_npp(npp), m_go(NULL), m_theMan(theMan)
{
    m_go = (BPGenericObject *) BPGenericObject::getObject(npp);
    m_go->defineFunction(s_onEnter, this);
    m_go->defineFunction(s_onExit, this);
    m_go->defineFunction(s_onDrop, this);
}

#define CHECK_RETURN(_x, _err) if (!(_x)) { oArgs.clear(); BPLOG_ERROR((_err)); return false; }

// a function to extract and verify drop contents from trusted javascript
static bool
extractPathArguments(NPP npp,
                     const NPVariant* args, uint32_t argCount,
                     std::vector<bp::file::Path> & oArgs)
{
    // we expect javascript to call us with an array of arrays.
    // each sub-array has exactly 2 elements, the first is a File object
    // the second is a string path
    CHECK_RETURN((argCount == 1 && npu::isArray(npp, args)),
                 "Trusted dnd JS invoked plugin without proper array arguments");

    // get the array length
    unsigned int arrLen = 0;
    CHECK_RETURN(npu::getArrayLength(npp, args[0].value.objectValue, &arrLen),
                 "Failed to get array length");

    for (unsigned int i = 0; i < arrLen; i++)
    {
        std::string name;
        std::string url;
        size_t size;

        NPVariant arElem;
        arElem.type = NPVariantType_Void;
        
        // extract array element
        CHECK_RETURN(npu::getArrayElement(npp, args[0].value.objectValue,
                                          i, &arElem),
                     "couldn't access array element");
        
        // each element should be an array of length 2
        {
            unsigned int shouldBeTwo = 0;
            CHECK_RETURN((!npu::isArray(npp, &arElem) ||
                          npu::getArrayLength(npp, arElem.value.objectValue, &shouldBeTwo)),
                         "invalid array member");
            CHECK_RETURN((shouldBeTwo == 2), "invalid array member cardinality");
        }
        
        // extract File and url
        NPVariant fileVar, urlVar;
        fileVar.type = urlVar.type = NPVariantType_Void;
        
        CHECK_RETURN(npu::getArrayElement(npp, arElem.value.objectValue, 0, &fileVar),
                     "couldn't extract file");
        CHECK_RETURN(npu::getArrayElement(npp, arElem.value.objectValue, 1, &urlVar),
                     "couldn't extract url");
        
        // pull out url string
        {
            NPAPIVariant vObj(&urlVar);
            CHECK_RETURN(vObj.getStringValue(url), "url not string");
        }
        
        CHECK_RETURN(fileVar.type == NPVariantType_Object, "file not object");
        
        // verify that url is a file:// url
        bp::file::Path path = bp::file::pathFromURL(url);
        if (path.empty()) {
            BPLOG_INFO_STRM("ignore non-file url " << url);
            continue;
        }

        // pull out filename
        {
            NPVariant fileName;
            fileName.type = NPVariantType_Void;
            CHECK_RETURN(npu::getElementProperty(npp, &fileVar, "fileName", &fileName),
                         "missing fileName");

            CHECK_RETURN(fileName.type == NPVariantType_String, "fileName not String (I)");

            NPAPIVariant vObj(&fileName);
            CHECK_RETURN(vObj.getStringValue(name), "fileName not string (II)");
        }

        // pull out file size
        {
            NPVariant fileSize;
            fileSize.type = NPVariantType_Void;
            CHECK_RETURN(npu::getElementProperty(npp, &fileVar, "fileSize", &fileSize),
                         "missing fileSize");
            if (fileSize.type == NPVariantType_Int32) {
                size = fileSize.value.intValue;                
            } else {
                CHECK_RETURN(fileSize.type == NPVariantType_Double, "fileSize not number");
                size = (size_t) fileSize.value.doubleValue;                
            }
        }

        // finally!  now we can validate the data
        BPLOG_INFO_STRM("(" << name << "/" << size << ") - " << path);

        // Validate name/size/path all are in harmony.
        // Links must be chased first.  Dirs have zero size according to 
        // boost, so don't check them. 
        CHECK_RETURN(name.compare(bp::file::utf8FromNative(path.filename())) == 0,
                     "name mismatch");
        bp::file::Path resolvedPath = path;
        if (bp::file::isLink(path)) {
            CHECK_RETURN(bp::file::resolveLink(path, resolvedPath),
                         "unable to resolve link");
        }
        if (bp::file::isRegularFile(resolvedPath)) {
            CHECK_RETURN(bp::file::size(resolvedPath) == size,
                         "size mismatch");
        }
            
        oArgs.push_back(path);
    }

    return true;
}

bp::Object *
Html5DropManager::DropTargetContext::invoke(
    const std::string & funcName, const NPVariant* args,
    uint32_t argCount)
{
    if (!s_onEnter.compare(funcName)) {
        // now we must extract file arguments...
        m_theMan->m_listener->onHover(m_name, true);
    } else if (!s_onExit.compare(funcName)) {
        leave(false);
        m_theMan->m_listener->onHover(m_name, false);
    } else if (!s_onDrop.compare(funcName)) {
        std::vector<bp::file::Path> paths;
        if (!extractPathArguments(m_npp, args, argCount, paths)) {
            BPLOG_ERROR("error extracting paths passed from trusted JavaScript on drag enter");
        } else {
            BPLOG_INFO_STRM("Got " << paths.size() << "dropped paths");
            enter(paths);
        }
        m_theMan->m_listener->onHover(m_name, false);
        bp::Object* dropList = dropItems();
        m_theMan->m_listener->onDrop(m_name, dropList);
        delete dropList;
    } else {
        BPLOG_ERROR_STRM("got unexpected call from javascript: " << funcName);
    }
    return NULL;
}

Html5DropManager::DropTargetContext::~DropTargetContext()
{
    delete m_go;
}
