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
 * An abstraction around the capturing standard error and standard output
 * and redirecting them into the BrowserPlus logging system.
 *
 * First introduced by Lloyd Hilaiel on 2009/01/20
 * Copyright (c) 2009 Yahoo!, Inc. All rights reserved.
 */

#ifndef __OUTPUTREDIRECTOR_H__
#define __OUTPUTREDIRECTOR_H__

class OutputRedirector 
{
  public:
    OutputRedirector();

    // upon invocation, stdout and stderr will be redirected to
    // BrowserPlus's logging infrastructure, and a thread will be
    // spawned who's responsibility it is to dump output from these
    // file descriptors to BPLOG

    // may only be called once per process, and everything will be torn
    // down once the destructor is called.
    void redirect();
    
    ~OutputRedirector();    
};

#endif
