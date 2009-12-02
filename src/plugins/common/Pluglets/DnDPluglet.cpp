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

#include "DnDPluglet.h"
#include "BPUtils/BPLog.h"
#include "PluginCommonLib/CommonErrors.h"


// win32 file systems tend to be much slower than osx, on win32 we'll
// have a limit of 1k files dropped (YIB-2077636)
#ifdef WIN32
#define DEFAULT_DROP_LIMIT 1000
#else
#define DEFAULT_DROP_LIMIT 10000
#endif
#define _MAKESTRING(x) #x
#define MAKESTRING(x) _MAKESTRING(x)


////////////////////////////////////////////////////////////
// Pluglet Introspection Support

BPArgumentDefinition s_addArguments[] = {
    {
        "id",
        "The 'id' of the registered DOM element to operate on",
        BPTString,
        true
    },
    {
        "mimeTypes",
        "A list of mimetypes to filter against.  Only items which "
        "match one of these mimetypes will be accepted.  Default "
        "is empty (all dropped items will be accepted).",
        BPTList,
        false
    },
    {
        "includeGestureInfo",
        "Should selection gesture information be included in the argument "
        "to the 'drop' callback?  Default is false.  If false, the argument "
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
        "Maximum number of items which will be included in a drop. "
		"Default is " MAKESTRING(DEFAULT_DROP_LIMIT),
        BPTInteger,
        false
    }
};

BPArgumentDefinition s_idArgument[] = {
    {
        "id",
        "The 'id' of the DOM element to operate on",
        BPTString,
        true
    }
};

BPArgumentDefinition s_attachArguments[] = {
    {
        "id",
        "The 'id' of the registered DOM element to which you wish to attach ",
        BPTString,
        true
    },
    {
        "hover",
        "A function that will be invoked when the user hovers over "
        "the drop target.  Argument is a boolean which when true means "
        "the user has entered the region, and when false means they have "
        "exited.",
        BPTCallBack,
        false
    },
    {
        "drop",
        "A function that will be invoked when the user drops files on "
        "your drop target.  Arguments to the callback vary depending on "
        "whether 'includeGestureInfo' was set true for the target. "
        "See the documentation for the 'includeGestureInfo' argument "
        "to 'AddDropTarget()'",
        BPTCallBack,
        false
    }
};

BPArgumentDefinition s_enableArguments[] = {
    {
        "id",
        "The 'id' of the registered DOM element for which you wish to "
        "enable/disable drag/drop activity.",
        BPTString,
        true
    },
    {
        "enable",
        "A boolean indicating whether activity should be enabled (true) "
        "or disabled (false).",
        BPTBoolean,
        true
    }
};

BPFunctionDefinition s_dndFunctions[] = {
    {
        "AddDropTarget",
        "Starts monitoring drag/drop activity for the specified element.",
        sizeof(s_addArguments)/sizeof(s_addArguments[0]),
        s_addArguments
    },
    {
        "AttachCallbacks",
        "AttachCallbacks to a registered drop target.  This function "
        "will not return until RemoveDropTarget is called, so it should "
        "not be invoked synchronously.",
        sizeof(s_attachArguments)/sizeof(s_attachArguments[0]),
        s_attachArguments
    },
    {
        "RemoveDropTarget",
        "Stop monitoring an element for drag/drop activity.",
        sizeof(s_idArgument)/sizeof(s_idArgument[0]),
        s_idArgument
    },
    {
        "EnableDropTarget",
        "Enable/disable an element for drag/drop activity.  AddDropTarget "
        "must have already been called for the element.",
        sizeof(s_enableArguments)/sizeof(s_enableArguments[0]),
        s_enableArguments
    },
    {
        "ListTargets",
        "Returns a list of the ids of the currently registered drop targets.",
        0,
        NULL
    }
};


// a description of this pluglet.
static BPCoreletDefinition s_dndPlugletDef = {
    "DragAndDrop",
    1, 0, 1,
    "Allow drag and drop of files from desktop to web browser.",
    sizeof(s_dndFunctions)/sizeof(s_dndFunctions[0]),
    s_dndFunctions
};


DnDPluglet::DnDPluglet(BPPlugin* plugin,
                       IDropManager* dropMgr)
    : Pluglet(plugin), m_dropMgr(dropMgr)
{
    m_desc.fromBPCoreletDefinition(&s_dndPlugletDef);

    if (m_dropMgr) {
        m_dropMgr->registerDropListener(this);
    }
}


DnDPluglet::~DnDPluglet()
{
}


void
DnDPluglet::execute(unsigned int tid,
                    const char* function,
                    const bp::Object* arguments,
                    bool syncInvocation,
                    plugletExecutionSuccessCB successCB,
                    plugletExecutionFailureCB failureCB,
                    plugletInvokeCallbackCB   callbackCB,
                    void* callbackArgument)
{
    if (!strcmp("AddDropTarget", function) ||
        !strcmp("RemoveDropTarget", function) ||
        !strcmp("AttachCallbacks", function) ||
        !strcmp("EnableDropTarget", function)) {
        if (!arguments->has("id", BPTString)) {
            failureCB(callbackArgument, tid, pluginerrors::InvalidParameters, NULL);
            return;
        }
        std::string id = ((bp::String*) arguments->get("id"))->value();
        
        if (!strcmp("AddDropTarget", function)) {
            // have we already got a target of this name?
            if (m_targets.find(id) != m_targets.end()) {
                failureCB(callbackArgument, tid,
                          "DnD.alreadyRegistered",
                          "This id is already registered as a drop target");
            } else {
                std::set<std::string> mimetypes;
                if (arguments->has("mimeTypes", BPTList)) {
                    const bp::List* l = (const bp::List*) arguments->get("mimeTypes");
                    for (unsigned int i = 0; i < l->size(); i++) {
                        const bp::String* s = dynamic_cast<const bp::String*>(l->value(i));
                        if (s) {
                            mimetypes.insert(s->value());
                        }
                    }
                } 
                bool gestureInfo = false;
                if (arguments->has("includeGestureInfo", BPTBoolean)) {
                    gestureInfo = ((bp::Bool*) arguments->get("includeGestureInfo"))->value();
                }
                unsigned int limit = DEFAULT_DROP_LIMIT;
                if (arguments->has("limit", BPTInteger)) {
                    limit = (unsigned int) (((bp::Integer*) arguments->get("limit"))->value());
                }
                bp::Bool rv(m_dropMgr->addTarget(id, mimetypes, 
                                                 gestureInfo, limit));
                m_targets[id] = DnDCallbackInfo();
                successCB(callbackArgument, tid, &rv);
            }
            
        } else if (!strcmp("RemoveDropTarget", function)) {
            // have we already got a target of this name?
            tTargetMap::iterator it = m_targets.find(id);
            if (it == m_targets.end()) {
                failureCB(callbackArgument, tid,
                          "DnD.notRegistered",
                          "This id is not registered as a drop target");
            } else {
                m_targets.erase(it);
                bp::Bool rv(m_dropMgr->removeTarget(id));
                successCB(callbackArgument, tid, &rv);
            }
            
        } else if (!strcmp("AttachCallbacks", function)) {
            // allowing sync invocation of AttachCallbacks would lock up
            // the browser.  (YIB-1609983)
            if (syncInvocation) {
                failureCB(callbackArgument, tid,
                          "DnD.invalidInvocation",
                          "AttachCallbacks may not be called synchronously");
                return;
            }
            
            tTargetMap::iterator it = m_targets.find(id);
            if (it == m_targets.end()) {
                failureCB(callbackArgument, tid,
                          "DnD.notRegistered",
                          "This id is not registered as a drop target");
            } else {
                BPCallBack hover = 0, drop = 0;
                if (arguments->has("hover", BPTCallBack)) {
                    hover = ((bp::CallBack*) arguments->get("hover"))->value();
                }
                if (arguments->has("drop", BPTCallBack)) {
                    drop = ((bp::CallBack*) arguments->get("drop"))->value();
                }
                
                it->second.hoverCB = hover;
                it->second.dropCB = drop;
                it->second.tid = tid;
                it->second.callback = callbackCB;
                it->second.callbackArg = callbackArgument;
                // no return!!!
            }

        } else if (!strcmp("EnableDropTarget", function)) {
            tTargetMap::iterator it = m_targets.find(id);
            if (it == m_targets.end()) {
                failureCB(callbackArgument, tid,
                          "DnD.notRegistered",
                          "This id is not registered as a drop target");
            } else {
                bool enable = false;
                if (arguments->has("enable", BPTBoolean)) {
                    enable = ((bp::Bool*) arguments->get("enable"))->value();
                }
                bp::Bool rv(m_dropMgr->enableTarget(id, enable));
                successCB(callbackArgument, tid, &rv);
            }
        }
            
    } else if (!strcmp("ListTargets", function)) {
        bp::List l;
        tTargetMap::iterator it;
        for (it = m_targets.begin(); it != m_targets.end(); it++) {
            l.append(new bp::String(it->first));
        }
        successCB(callbackArgument, tid, &l);        
        
    } else {
        std::string s("unknown drag/drop function " + std::string(function) + " called");
        bp::String str(s);
        successCB(callbackArgument, tid, &str);
    }
}
    

const bp::service::Description *
DnDPluglet::describe()
{
    return &m_desc;
}


////////////////////////////////////////////////////////////
// IDropListener Methods

void
DnDPluglet::onHover(const std::string& id, 
                    bool hover)
{
    BPLOG_INFO_STRM("hover " << (hover ? "on" : "off") <<
                    " for '" << id << "'");

    tTargetMap::iterator it;
    for (it = m_targets.begin(); it != m_targets.end(); it++) {
        if (!id.compare(it->first)) {
            if (it->second.callback) {
                bp::Bool val(hover);
                it->second.callback(it->second.callbackArg,
                                    it->second.tid,
                                    it->second.hoverCB,
                                    &val);
            }
        }
    }
}


void
DnDPluglet::onDrop(const std::string& id,
                   const bp::Object* items)
{
    std::string s = items->toPlainJsonString(true);
    BPLOG_INFO_STRM(id << " get drop " << s);
    
    if (dynamic_cast<const bp::List*>(items) == NULL
        && dynamic_cast<const bp::Map*>(items) == NULL) {
        BPLOG_ERROR_STRM("dropped items " << s << " not a list or map");
        return;
    }

    tTargetMap::iterator it;
    for (it = m_targets.begin(); it != m_targets.end(); it++) {
        if (!id.compare(it->first)) {
            if (it->second.callback) {
                it->second.callback(it->second.callbackArg,
                                    it->second.tid,
                                    it->second.dropCB,
                                    items);
            }
        }
    }
}
