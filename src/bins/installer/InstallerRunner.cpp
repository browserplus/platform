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
 * Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/**
 * A simple proxy abstraction which runs the Installer on a separate
 * thread.
 */

#include "InstallerRunner.h"

InstallerRunner::InstallerRunner()
    : m_listener(NULL)
{
}
    
InstallerRunner::~InstallerRunner()
{
    m_installerThread.join();
}

void
InstallerRunner::setListener(bp::install::IInstallerListener * listener)
{
    m_listener = listener;
}

static void *
installerThreadFunc(void * cookie)
{
    bp::install::Installer * i = (bp::install::Installer *) cookie;
    i->run();
    return NULL;
}

// allocate the underlying installer and get it running
void
InstallerRunner::start(const bp::file::Path& dir,
                       bool deleteWhenDone)
{
    m_installer.reset(new bp::install::Installer(dir, deleteWhenDone));
    m_installer->setListener(shared_from_this());
    
    // now start the thread
    m_installerThread.run(installerThreadFunc,
                          (void *) m_installer.get());
}


struct IRMessage
{
    typedef enum { E_Status, E_Error, E_Progress } Type;

    IRMessage(Type t, std::string m, unsigned int p)
       : type(t), msg(m), pct(p) 
    {
    }
    Type type;
    std::string msg;
    unsigned int pct;
};

void
InstallerRunner::onStatus(const std::string& msg)
{
    hop((void *) new IRMessage(IRMessage::E_Status, msg, 0));
}

void
InstallerRunner::onError(const std::string& msg)
{
    hop((void *) new IRMessage(IRMessage::E_Error, msg, 0));
}

void
InstallerRunner::onProgress(unsigned int pct)
{
    hop((void *) new IRMessage(IRMessage::E_Progress, std::string(), pct));
}

void
InstallerRunner::onHop(void * context)
{
    IRMessage * irm = (IRMessage *) context;
    if (m_listener) {
        switch (irm->type) {
            case IRMessage::E_Status:
                m_listener->onStatus(irm->msg);
                break;
            case IRMessage::E_Error:
                m_listener->onError(irm->msg);
                break;
            case IRMessage::E_Progress:
                m_listener->onProgress(irm->pct);
        }
    }
    delete irm;
}
