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
 *  bpthread.h -- POSIX like abstractions around OS threads.
 *
 *  Adapted from WUF Tehcnologies threading abstraction.
 *
 *  Written by Lloyd Hilaiel, 2005 & 2007
 *  Copywrite Yahoo! Inc. 2007
 */

#include "bpthread.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <assert.h>

using namespace bp::thread;

static void blockSignal(int cig)
{
  sigset_t ss;

  sigemptyset(&ss);
  sigaddset(&ss, cig);
  pthread_sigmask(SIG_BLOCK, &ss, NULL);
}

Thread::Thread()
{
    m_osSpecific = (void *) calloc(1, sizeof(pthread_t));
}

Thread::~Thread()
{
    free(m_osSpecific);
    m_osSpecific = NULL;
}

struct barDat_t {
  StartRoutine runme;
  void *arg;
};

static void *blockAndRun(void *arg1)
{
  struct barDat_t *bdptr = (struct barDat_t *) arg1;
  StartRoutine runme = bdptr->runme;
  void *arg = bdptr->arg;

  blockSignal(SIGINT);
  free(bdptr);

  return runme(arg);
}

bool
Thread::run(StartRoutine startFunc, void * cookie)
{
    int rsz = -1;
    struct barDat_t *bdptr;

    bdptr = (barDat_t *) malloc(sizeof(struct barDat_t));
    assert(bdptr != NULL);

    bdptr->runme = startFunc;
    bdptr->arg = cookie;
    rsz = pthread_create((pthread_t *) m_osSpecific, NULL,
                         blockAndRun, bdptr);

    if (rsz != 0) free(bdptr);

    return (rsz == 0);
}

void
Thread::detach()
{
    pthread_detach(*((pthread_t *) m_osSpecific));
}

void
Thread::join()
{
    pthread_join(*((pthread_t *) m_osSpecific), NULL);
}

unsigned int
Thread::ID()
{
    return (unsigned int) *((pthread_t *) m_osSpecific);
}

unsigned int
Thread::currentThreadID()
{
    return (unsigned int) pthread_self();
}
