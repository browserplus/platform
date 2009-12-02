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

/*
 * An abstraction which is employed by a forked process to 
 * load a specified service and communicate back with the forker.
 *
 * First introduced by Lloyd Hilaiel on 2009/01/14
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#ifndef __SPAWNEDSERVICEPROCESS_H__
#define __SPAWNEDSERVICEPROCESS_H__

namespace ServiceRunner 
{
    /**
     * ServiceRunner::runProcess() encapsulates the core logic
     * for a Service Process, that is a process spawned using
     * ServiceRunner::Controller.  Said spawned process simply
     * should call runService(), when it exits, the calling process
     * should exit.
     *
     * Inputs:  Before calling run the caller should change into
     * the directory containing the loaded service.  CWD is in effect
     * an input to the ServiceRunner.
     */
    bool runServiceProcess(int argc, const char ** argv);
};

#endif
