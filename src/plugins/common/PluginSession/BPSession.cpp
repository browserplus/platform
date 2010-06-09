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
 *  BPSession.cpp
 *
 *  Created by Lloyd Hilaiel
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include <boost/scoped_ptr.hpp>

#include "BPSession.h"
#include "BPHandleMapper.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bperrorutil.h"
#include "CommonErrors.h"



BPSession::BPSession(BPPlugin* pPlugin)
    : m_plugin(pPlugin),
      m_initialized(false)
{
    // initialize the thread hopper so we can get back to this thread
    // of control later.
    m_threadHopper.initializeOnCurrentThread();

    m_plugletRegistry = new PlugletRegistry;
    
    // register all the pluglets
    m_plugletRegistry->registerPluglets(m_plugin->createPluglets("DragAndDrop"));
    m_plugletRegistry->registerPluglets(m_plugin->createPluglets("FileBrowser"));
    m_plugletRegistry->registerPluglets(m_plugin->createPluglets("FileSave"));
    m_plugletRegistry->registerPluglets(m_plugin->createPluglets("Log"));

    // allocate a protocol handle 
    m_protoHand = BPAlloc();
    BPLOG_DEBUG_STRM("\t-> Allocated m_protoHand " <<
                     BP_HEX_MANIP << long(m_protoHand));

    // 2008nov11 dg: currently on IE this ctor is called from Ax
    // FinalConstruct, which is prior to browser "attach".
    // You currently can't call anything here that requires access to
    // the browser object model.  This could change if we moved the
    // instantiation of BPSession on IE to post browser attach.
//  BPLOG_INFO_STRM("New session allocated for user agent: "
//                   << pPlugin->getUserAgent());
    BPLOG_INFO("New session allocated.");

    // actual connection to bpcore occurs in BPSession::initialize()
}

BPSession::~BPSession()
{
    BPLOG_INFO_STRM("-> Teardown BPSession object " <<
                    "(" << BP_HEX_MANIP << long(this) << ")");

    if (m_protoHand != NULL) {
        BPFree(m_protoHand);
        m_protoHand = NULL;
    }

    BPASSERT(m_plugletRegistry != NULL);
    delete m_plugletRegistry;
    m_plugletRegistry = NULL;
}



bool
BPSession::generateErrorReturn(const char* cszError,
                               const char* cszVerboseError,
                               plugin::Variant* pvtRet) const
{
    if (cszError == NULL) cszError = pluginerrors::UnknownError;
            
    bp::Map errorMap;
    errorMap.add("success", new bp::Bool(false));
    errorMap.add("error", new bp::String(cszError));
    if (cszVerboseError) {
        errorMap.add("verboseError", new bp::String(cszVerboseError));
    }

    return plugin().evaluateJSON(&errorMap, pvtRet);
}


bool
BPSession::generateSuccessReturn(const bp::Object& value,
                                 plugin::Variant* pvtRet) const
{
    bp::Map successMap;
    successMap.add("success", new bp::Bool(true));
    successMap.add("value", value.clone());
    return plugin().evaluateJSON(&successMap, pvtRet);
}


bool
BPSession::findLoadedService(const std::string & service,
                             const std::string & version,
                             const std::string & minversion,
                             bp::service::Description & oDesc)
{
    std::list<bp::service::Description>::iterator it;
    bp::ServiceVersion got;
    bp::ServiceVersion wantver;
    bp::ServiceVersion wantminver;
    bool found = false;

    if (!wantver.parse(version.c_str()) || !wantminver.parse(minversion)) {
        return false;
    }

    // XXX
    for (it = m_loadedServices.begin(); it != m_loadedServices.end(); it++) {
        BPLOG_INFO_STRM("loaded service: " << it->name() << " / " << it->versionString());
    }

    for (it = m_loadedServices.begin(); it != m_loadedServices.end(); it++)
    {
        std::string curService;
        std::string curVersion;

        curService = it->name();
        curVersion = it->versionString();

        bp::ServiceVersion current;
        if (!current.parse(curVersion)) continue; /** can't parse!? */

        if (!curService.compare(service)) {
            if (bp::ServiceVersion::isNewerMatch(current, got,
                                                 wantver, wantminver)) {
                found = true;
                got = current;
                oDesc = *it;
            }
        }
    }

    return found;
}


void
BPSession::addTransaction(BPTransaction * t)
{
    if (t != NULL) {
        m_activeTransactions.push_back(t);
    }
}

void
BPSession::removeTransaction(BPTransaction * t)
{
    if (t == NULL) return;

    std::list<BPTransaction *>::iterator it;
    for (it = m_activeTransactions.begin();
         it != m_activeTransactions.end();
         it++)
    {
        if ((*it) == t) break;
    }
    if (it != m_activeTransactions.end()) {
        m_activeTransactions.erase(it);
    }
    delete t;
}

BPTransaction *
BPSession::findTransactionByProtoID(unsigned int id)
{
    std::list<BPTransaction *>::iterator it;
    for (it = m_activeTransactions.begin();
         it != m_activeTransactions.end();
         it++)
    {
        if ((*it)->m_bpProtoTransactionID == id) break;
    }
    if (it != m_activeTransactions.end()) {
        return *it;
    }

    return NULL;
}

BPTransaction *
BPSession::findTransactionByTID(unsigned int id)
{
    std::list<BPTransaction *>::iterator it;
    for (it = m_activeTransactions.begin();
         it != m_activeTransactions.end();
         it++)
    {
        if ((*it)->tid() == id) break;
    }

    if (it != m_activeTransactions.end()) {
        return *it;
    }

    return NULL;
}


bool
BPSession::notInitialized(const plugin::Object* callback,
                          plugin::Variant* result)
{
    if (!m_initialized) {
        if (callback) {
            plugin::Variant* err = plugin().allocVariant();
            plugin::Variant* result2 = plugin().allocVariant();
            
            if (!generateErrorReturn(pluginerrors::NotInitialized,
                                     NULL, err))
            {
                err->clear();
            }

            (void) plugin().callJsFunction(callback, &err, 1, result2);

            plugin().freeVariant(err);
            plugin().freeVariant(result2);    
        }
        else
        {
            if (!generateErrorReturn(pluginerrors::NotInitialized,
                                     NULL, result))
            {
                result->clear();
            }
        }
        return true;
    }

    return false;
}


bool
BPSession::variantToBPObject( const plugin::Variant* input,
                              BPTransaction* transaction,
                              bp::Object*& output ) const
{
    output = NULL;
    if (input == NULL || input->isVoid()) {
        output = NULL;
    } else if (input->isNull()) {
        output = new bp::Null;
    } else if (input->isBoolean()) {
        bool bVal;
        input->getBooleanValue(bVal);
        output = new bp::Bool(bVal);
    } else if (input->isInteger()) {
        int nVal;
        input->getIntegerValue(nVal);
        output = new bp::Integer(nVal);
    } else if (input->isDouble()) {
        double dVal;
        input->getDoubleValue(dVal);
        output = new bp::Double(dVal);
    } else if (input->isString()) {
        std::string sVal;
        input->getStringValue(sVal);
        output = new bp::String(sVal);
    } else if (input->isObject()) {
        plugin::Object* p = NULL;
        input->getObjectValue(p);
        boost::scoped_ptr<plugin::Object> oVal(p);
           
        if (plugin().isJsFunction(oVal.get())) {
            plugin::Object * callback = oVal.get();
            unsigned int cid = transaction->addCallback(callback);
            output = new bp::CallBack(cid);
        } else if (plugin().isJsArray(oVal.get())) {
            bp::List * l = new bp::List;

            plugin::Object * arr = oVal.get();

            // Get array length.
            int nArrLen;
            if (!plugin().getArrayLength( arr, nArrLen )) {
                delete l;
                return false;
            }
                    
            // now iterate through the array.
            for (int i = 0; i < nArrLen; i++)
            {
                plugin::Variant* pvtElem = plugin().allocVariant();
                if (plugin().getArrayElement(arr, i, pvtElem))
                {
                    bp::Object * pbpObj = NULL;
                    if (!variantToBPObject(pvtElem, transaction, pbpObj)) {
                        delete l;
                        return false;
                    }
                    if (pbpObj) l->append(pbpObj);
                    plugin().freeVariant(pvtElem);
                }
                else
                {
                    plugin().freeVariant(pvtElem);
                    break;
                }
            }
            output = l;
        } else {
            // now we know it's either a Path, or a Map 

            // iterate through all values
            bp::Map * m = new bp::Map;

            // get the keys
            std::vector<std::string> keys;
            if (!plugin().enumerateProperties(oVal.get(), keys)) {
                delete m;
                return false;
            }
            
            for (unsigned int i = 0; i < keys.size(); i++) {
                plugin::Variant* val = plugin().allocVariant();
                if (!plugin().getProperty(oVal.get(), keys[i], val)) {
                    delete m;
                    return false;
                }
                bp::Object * value = NULL;
                if (!variantToBPObject(val, transaction, value)) {
                    delete m;
                    return false;
                }
                if (value) 
                    m->add(keys[i].c_str(), value);
                plugin().freeVariant(val);
            }

            // if this map has three special keys, it's really a path
            if (m->has(BROWSERPLUS_HANDLETYPE_KEY, BPTString) &&
                (m->has(BROWSERPLUS_HANDLEID_KEY, BPTInteger) ||
                 m->has(BROWSERPLUS_HANDLEID_KEY, BPTDouble)) &&
                m->has(BROWSERPLUS_HANDLENAME_KEY, BPTString) &&
                m->has(BROWSERPLUS_HANDLEMIMETYPE_KEY, BPTList))
            {
                std::string type = 
                    dynamic_cast<const bp::String *>(m->get(
                        BROWSERPLUS_HANDLETYPE_KEY))->value();
                int id;
                if (m->has(BROWSERPLUS_HANDLEID_KEY, BPTDouble)) { 
                    id = (int) dynamic_cast<const bp::Double *>(m->get(
                        BROWSERPLUS_HANDLEID_KEY))->value();
                } else {
                    id = (int) dynamic_cast<const bp::Integer *>(m->get(
                        BROWSERPLUS_HANDLEID_KEY))->value();
                }
                std::string name = 
                    dynamic_cast<const bp::String *>(m->get(
                        BROWSERPLUS_HANDLENAME_KEY))->value();
                // size
                long size = 0;
                if (m->has(BROWSERPLUS_HANDLESIZE_KEY, BPTDouble)) { 
                    size = (long) dynamic_cast<const bp::Double *>(m->get(
                        BROWSERPLUS_HANDLESIZE_KEY))->value();
                } else if (m->has(BROWSERPLUS_HANDLESIZE_KEY, BPTInteger)) { 
                    size = (long) dynamic_cast<const bp::Integer *>(m->get(
                        BROWSERPLUS_HANDLESIZE_KEY))->value();
                }
                // mimetypes
                std::set<std::string> mimeTypes;
                const bp::List* l = 
                    dynamic_cast<const bp::List *>(m->get(
                        BROWSERPLUS_HANDLEMIMETYPE_KEY));
                if (l) {
                    for (unsigned int i = 0; i < l->size(); ++i) {
                        const bp::String* s = 
                            dynamic_cast<const bp::String*>(l->value(i));
                        if (s) {
                            mimeTypes.insert(s->value());
                        }
                    }
                }

                bp::file::Path path = BPHandleMapper::handleValue(
                    BPHandle(type, id, name, size, mimeTypes));

                delete m;

                output = new bp::Path(path);
            }
            else
            {
                output = m;
            }
        }
    } else {
        return false;
    }

    return true;
}

PlugletRegistry *
BPSession::getPlugletRegistry()
{
    return m_plugletRegistry;
}

bool
BPSession::invokeCallbackWithError(const char * error,
                                   const char * verboseError,
                                   const plugin::Object* callback)
{
    plugin::Variant * trash = plugin().allocVariant();
    plugin::Variant * arg = plugin().allocVariant();
    generateErrorReturn(error, verboseError, arg);
    bool rv = plugin().callJsFunction(callback, &arg, 1, trash);
    plugin().freeVariant(arg);
    plugin().freeVariant(trash);

    return rv;
}


