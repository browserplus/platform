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
 * CoreletInstance
 *
 * An instantiated corelet instance upon which functions may be
 * invoked.
 */

#include "CoreletInstance.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bptypeutil.h"

using namespace std;
using namespace std::tr1;


CoreletInstance::CoreletInstance(weak_ptr<CoreletExecutionContext> c)
{
    m_context = c;
}

CoreletInstance::~CoreletInstance()
{
}

// This structure is an in memory representation of data that flows from 
// the service's execution thread to the main thread
struct DataToCore
{
    typedef enum {
        T_InvokeCallback,
        T_PromptUser,
        T_ExecutionComplete,
        T_ExecutionFailure
    } TransactionType;
    
    DataToCore(TransactionType t) : type(t), tid(0), promptId(0)
    {
    }

    // the type of transaction
    TransactionType type;

    // transaction id
    unsigned int tid;

    // bp::Object data, used in several different transaction types
    shared_ptr<bp::Object> data;

    // elements specific to T_PromptUser
    unsigned int promptId;
    bp::file::Path pathToHTMLDialog;

    // elements specific to T_ExecutionFailure
    std::string error;
    std::string verboseError;
};

void
CoreletInstance::sendComplete(unsigned int tid,
                              const bp::Object & results)
{
    DataToCore * dtc = new DataToCore(DataToCore::T_ExecutionComplete);
    dtc->tid = tid;
    dtc->data.reset(results.clone());
    hop((void *) dtc);
}

void
CoreletInstance::sendFailure(unsigned int tid, const std::string & error,
                             const std::string & verboseError)
{
    DataToCore * dtc = new DataToCore(DataToCore::T_ExecutionFailure);
    dtc->tid = tid;
    dtc->error.append(error);
    dtc->verboseError.append(verboseError);
    hop((void *) dtc);
}

void
CoreletInstance::invokeCallback(unsigned int tid,
                                const bp::Object & results)
{
    DataToCore * dtc = new DataToCore(DataToCore::T_InvokeCallback);
    dtc->tid = tid;
    dtc->data.reset(results.clone());
    hop((void *) dtc);
}


void
CoreletInstance::sendUserPrompt(
    unsigned int cookie,
    const bp::file::Path& pathToHTMLDialog,
    const bp::Object * arguments)
{
    DataToCore * dtc = new DataToCore(DataToCore::T_PromptUser);
    dtc->promptId = cookie;
    dtc->pathToHTMLDialog = pathToHTMLDialog;
    dtc->data.reset(arguments->clone());
    hop((void *) dtc);
}

void
CoreletInstance::onHop(void * ctx)
{
    DataToCore * dtc = (DataToCore *) ctx;

    if (dtc == NULL) {
        BPLOG_ERROR_STRM("received null data");
        return;
    }

    shared_ptr<CoreletExecutionContext> context = m_context.lock();

    // is the context null?  if so we must abort
    if (context == NULL) {
        BPLOG_ERROR_STRM("service instance has null context, "
                         << "aborting transaction");
        return;
    }
    
    shared_ptr<ICoreletInstanceListener> listener = m_listener.lock();

    // is the listener null?  if so we must abort
    if (listener == NULL) {
        BPLOG_ERROR_STRM("service instance has null listener, "
                         << "aborting transaction");
        return;
    }

    // now let's act on it!
    switch (dtc->type) {
        case DataToCore::T_InvokeCallback: 
        {
            BPASSERT((dtc->data != NULL)
                   && (dtc->data.get()->type() == BPTMap));
            context->invokeCallback(dtc->tid,
                                    dynamic_cast<bp::Map *>(dtc->data.get()));
            break;
        }
        case DataToCore::T_PromptUser:
        {
            BPLOG_INFO_STRM("prompting user: " << dtc->promptId);

            context->promptUser(shared_from_this(), dtc->promptId,
                                dtc->pathToHTMLDialog,
                                dtc->data.get());
            break;
        }
        case DataToCore::T_ExecutionComplete: 
        {
            bp::Null n;
            bp::Object * obj = &n;
            if (dtc->data) obj = dtc->data.get();
            listener->executionComplete(dtc->tid, *obj);
            break;
        }
        case DataToCore::T_ExecutionFailure:
        {
            listener->executionFailure(dtc->tid, dtc->error,
                                       dtc->verboseError);            
            break;
        }
        default:
            BPLOG_ERROR_STRM("unhandled data type: " << dtc->type);
    }

    delete dtc;
}

void
CoreletInstance::setListener(
    weak_ptr<ICoreletInstanceListener> listener)
{
    m_listener = listener;
}
