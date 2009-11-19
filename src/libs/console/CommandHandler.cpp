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
 * CommandHandler.cpp
 * Callback functions that handle commands (\see CommandParser.h)
 *
 * Created by Lloyd Hilaiel on Mon Sep 6 2005.
 * Copyright (c) 2005-2009 Yahoo!, Inc. All rights reserved.
 */

#include "CommandHandler.h"

using namespace std;
using namespace std::tr1;


void
CommandHandler::setListener(weak_ptr<IHandlerListener> listener)
{
    m_listener = listener;
}

void
CommandHandler::onSuccess()
{
    shared_ptr<IHandlerListener> listener = m_listener.lock();
    if (listener) listener->onSuccess();
}

void
CommandHandler::onFailure()
{
    shared_ptr<IHandlerListener> listener = m_listener.lock();
    if (listener) listener->onFailure();
}


