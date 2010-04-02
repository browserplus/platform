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

#include "BPSession.h"
#include "BPProtocol/BPProtocol.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bperrorutil.h"
#include "PluginCommonLib/NativeUI.h"


struct PromptContext
{
    PromptContext() : session(NULL), tid(0), arguments(NULL) { }
    ~PromptContext() { if (arguments != NULL) delete arguments; }    

    BPSession * session;
    unsigned int tid;

    // common strings
    bp::Object * arguments;
    std::string pathToHTMLDialog;
};

void
BPSession::executeUserPrompt(void * cookie)
{
    PromptContext * pc = (PromptContext *) cookie;
    BPASSERT(pc == pc->session->m_prompts.front());
    bool rv = false;
    bp::Object * rez = NULL;
    std::string uagent = pc->session->m_plugin->getUserAgent();

    rv = bp::ui::HTMLPrompt(pc->session->plugin().getWindow(),
                            pc->pathToHTMLDialog,
                            uagent, 
                            pc->arguments,
                            &rez);

    pc->session->m_prompts.pop_front();
    BPDeliverUserResponse(pc->session->m_protoHand, pc->tid,
                          (rv ? rez->elemPtr() : NULL));
    if (rez) delete rez;
    delete pc;
}

void
BPSession::handleUserPrompt(void * cookie,
                            const char * pathToHTMLDialog,
                            const BPElement * arguments,
                            unsigned int tid)
{
    BPSession * session = (BPSession *) cookie;
    BPLOG_INFO("-- > got user prompt request < --");

    PromptContext * pc = new PromptContext;
    pc->session = session;
    pc->tid = tid;
    pc->arguments = NULL;
    pc->pathToHTMLDialog.append(pathToHTMLDialog);

    if (arguments != NULL) {
        pc->arguments = bp::Object::build(arguments);
    }

    pc->session->m_prompts.push_back(pc);
    if (pc->session->m_prompts.size() == 1) {
        executeUserPrompt(pc);
    }
}

void
BPSession::deliverUserPromptResults(void * cookie, const bp::Object * response)
{
    BPLOG_INFO("delivering user prompt results");
    PromptContext * pc = (PromptContext *) cookie;
    BPASSERT(pc->session->m_prompts.front() == cookie);
    pc->session->m_prompts.pop_front();

    BPDeliverUserResponse(pc->session->m_protoHand, pc->tid,
                          response ? response->elemPtr() : NULL);
    if (pc->session->m_prompts.size() > 0) {
        executeUserPrompt(pc->session->m_prompts.front());
    }
    delete pc;
    
}
