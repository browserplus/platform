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

/**
 * Copyright 2006-2008, Yahoo!
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *  3. Neither the name of Yahoo! nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 *  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 *  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include <ServiceAPI/bperror.h>
#include <ServiceAPI/bptypes.h>
#include <ServiceAPI/bpdefinition.h>
#include <ServiceAPI/bpcfunctions.h>
#include <ServiceAPI/bppfunctions.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

static const BPCFunctionTable* g_bpCoreFunctions;

#define SAYHELLO_FUNCNAME ((char*) "SayHello")

/**
 * a function called at the time a webpage calls a function on your
 * service for the first time, establishing a "session" that has a
 * lifetime tied to webpage lifetime. 
 *
 * Allocate will be called on a thread that is spun for this session,
 * so blocking isn't awesome, but it won't slow down other service
 * instances.
 *
 * Note, multiple sessions may exist simultaneously, each run on a
 * distinct thread.  Any static data must be protected.
 */
static int
myAllocate(void** instance,
           const BPString uri,
           const BPPath serviceDir,
           const BPPath dataDir,
           const BPPath tempDir,
           const BPString locale,
           const BPString userAgent,
           int clientPid)
{
    /* 'instance' is a pointer to context that you control.  You can
     * allocate whatever session specific context you want and
     * it will later be passed to your invoke and destroy functions */
    *instance = malloc(4);
    (*(unsigned int*)(*instance)) = 77;
    printf("BPPAllocate %p\n", *instance);
    printf("uri: %s\n", uri);
    printf("serviceDir: %s\n", serviceDir);
    printf("dataDir: %s\n", dataDir);
    printf("tempDir: %s\n", tempDir);
    printf("locale: %s\n", locale);
    printf("userAgent: %s\n", userAgent);
    printf("clientPid: %u\n", clientPid);
    return 0;
}


/**
 * a function called at the time a webpage is closed or reloaded.  Will
 * be invoked on the same thread as 'myAllocate' above.
 *
 * This function provides an opportunity to clean up any session specific
 * data that is allocated.
 */
static void
myDestroy(void* instance)
{
    printf("** (SampleService) BPPDestroy %p\n", instance);
    free(instance);
}


/**
 * a function called immediately before your service is unloaded. This
 * will occur immediately when the last webpage using your service is
 * closed.
 *
 * This function provides an opportunity to clean up any service wide
 * data that is allocated.
 */
static void
myShutdown(void)
{
    printf("** (SampleService) BPPShutdown\n");
}


/**
 * a function called exactly once immediately after your service
 * in installed on disk.  This is an opportunity to perform any
 * one-time setup.
 */
static int
myInstall(const BPPath serviceDir,
          const BPPath dataDir) 
{
    printf("** (SampleService) BPPInstall\n");
    printf("serviceDir = %s\n", serviceDir);
    printf("dataDir = %s\n", serviceDir);
    return 0;
}


/**
 * a function called exactly once immediately after your service
 * in installed on disk.  This is an opportunity to perform any
 * one-time setup.
 */
static int
myUninstall(const BPPath serviceDir,
            const BPPath dataDir) 
{
    printf("** (SampleService) BPPUninstall\n");
    printf("serviceDir = %s\n", serviceDir);
    printf("dataDir = %s\n", serviceDir);
    return 0;
}


/**
 * The implementation of a function.  This demonstrates how data is 
 * represented as it's transmitted into services, and how to return data.
 */
static void
helloWorldFunc(void* instance,
               unsigned int tid,
               const BPElement* elem)
{
    printf("** (SampleService) [%u] helloWorldFunc %p\n", tid, instance);
    printf("** (SampleService) called with %u arguments\n",
           elem->value.mapVal.size);    

    // extract argument
    const char* who = "unknown";
    
    if (elem->value.mapVal.size == 1) {
        who = elem->value.mapVal.elements[0].value->value.stringVal;
    }

    // build up our return value, this value will not have a lifetime
    // beyond the call to postResults()
    BPElement retval;
    retval.type = BPTString;
    std::string rv;
    rv.append("Welcome to the b+ world, ");
    rv += who;
    retval.value.stringVal = (char*) rv.c_str();

    // post results
    g_bpCoreFunctions->postResults(tid, &retval);
}


/**
 * Everytime a webpage executes a function on a service, your
 * invocation function will be called.  The primary role of the
 * invocation function is to dispatch and/or execute the function
 * requested by a webpage.
 */
static void
myInvoke(void* instance,
         const char* funcName,
         unsigned int tid,
         const BPElement* elem)
{
    if (!strcmp(SAYHELLO_FUNCNAME, funcName)) {
        helloWorldFunc(instance, tid, elem);
    } else {
        // this will never happen because the platform will perform
        // validation for us.  Because in the structures below we
        // define the interface to our service, which includes a
        // single function named 'SayHello', we will never
        // get a different funcName in normal operation.
        g_bpCoreFunctions->postError(tid, BPE_INTERNAL_ERROR, NULL);
    }
}


/**
 * Interfaces are defined in C structures.  The following static
 * memory describes the interface of this service and will be used
 * when dynamically building a callable javascript interface.
 * This information allows documentation to be build dynamically,
 * and allows the platform to perform argument validation for you.
 *
 * refer to bptypes.h for definitions of the various structures and
 * further documentation.
 */
static BPArgumentDefinition s_helloWorldArguments[] = {
    {
        (char*) "who",
        (char*) "Who we should welcome to the world",
        BPTString,
        BP_FALSE
    }
};

static BPFunctionDefinition s_myServiceFunctions[] = {
    {
        SAYHELLO_FUNCNAME,
        (char*) "A simple function to say hello world.",
        1,
        s_helloWorldArguments
    }
};

static BPServiceDefinition s_myServiceDef = {
    (char*) "SampleService",
    1, 0, 0,
    (char *) "A do-nothing service to see what writing a service is like.",
    1,
    s_myServiceFunctions
};


/**
 * initialize is called immediately after your service's dynamic library
 * is loaded.  It is called on the main application thread.  This function
 * should allocated any service wide resources required and should not
 * block.
 */
static const BPServiceDefinition*
myInitialize(const BPCFunctionTable* bpCoreFunctions,
             const BPPath serviceDir,
             const BPPath /* dependentDir -- not used in a standalone service */,
             const BPElement* /* dependentParams -- not used in a standalone service */)
{
    g_bpCoreFunctions = bpCoreFunctions;
    printf("** (SampleService) BPPInitialize (%p) (%s)\n", bpCoreFunctions,
           serviceDir);
    return &s_myServiceDef;
}


/* and finally, declare the entry point to the service, which is a
 * function table */
static BPPFunctionTable funcTable = {
    BPP_SERVICE_API_VERSION,
    myInitialize,
    myShutdown,
    myAllocate,
    myDestroy,
    myInvoke,
    NULL,             // cancel
    myInstall,        // install
    myUninstall       // uninstall
};


/** The only external symbol! All we do in this function is return a
 *  table containing the api version and pointers to functions. */
const BPPFunctionTable*
BPPGetEntryPoints(void)
{
    return &funcTable;
}
