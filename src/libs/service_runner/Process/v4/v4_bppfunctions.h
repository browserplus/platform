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
 * Written by Lloyd Hilaiel, on or around Fri May 18 17:06:54 MDT 2007 
 *
 * Overview:  
 *
 * Functions that must be implemented by the service are prefaced with
 * BPP and are located in this file.  
 */

#ifndef __BPPFUNCTIONS_V4_H__
#define __BPPFUNCTIONS_V4_H__

#include <v4_bptypes.h>
#include <v4_bpdefinition.h>
#include <v4_bpcfunctions.h>

namespace sapi_v4 {

/**
 * Initialize the service, called once at service load time.
 * 
 * \param coreFunctionTable  pointer to a structure of pointers to BPC
 *                           functions that the service may call through into
 *                           BPCore.
 * \param parameterMap       A map containing initialization parameters for
 *                           the service.  These parameters include:
 *
 *             'CoreletDirectory': The location on disk where the service
 *                 resides. 
 *                 
 */
typedef const BPCoreletDefinition * (*BPPInitializePtr)(
    const BPCFunctionTable * coreFunctionTable,
    const BPElement * parameterMap);

/**
 * Shutdown the service.  Called once at service unload time.
 * All allocated instances will have been deleted by the time
 * this function is called.
 */
typedef void (*BPPShutdownPtr)(void);

/**
 * Allocate a new service instance.  
 *
 * \param instance A void pointer that will be passed back to subsequent
 *                 calls to invoke and destroy.  This is an output parameter
 *                 that services may use to store instance-specific
 *                 context.
 * \param attachID The ID of the attachment this is associated with for
 *                 provider services (set in BPPAttach call), for
 *                 standalone services, always zero and may be ignored.
 * \param contextMap A map containing session specific context.
 *        'uri' is a string containing a URI of the current client. This is
 *              usually the full URL to the webpage that is interacting
 *              with browserplus.  In the case of a native local application
 *              interacting with browserplus it should be a URI with a
 *              method of 'bpclient' (i.e. bpclient://CLIENTIDENTIFIER').
 *        'corelet_dir' DEPRECATED, use service_dir instead.
 *        'service_dir' is an absolute path to the directory containing the
 *                   files distributed with the service.
 *        'data_dir' is a absolute path to where the service should store
 *                   any data that needs to persist.
 *        'temp_dir' is a directory that may be used for temporary
 *                   data.  it will be unique for every instance, and will
 *                   be purged when the instance is deallocated.
 *        'locale' The locale of the end user to which strings, if any,
 *                 should be localized.
 *        'userAgent' The client user-agent string.
 *        'clientPid' The process ID of the client.
 *
 * \return zero on success, non-zero on failure
 */
typedef int (*BPPAllocatePtr)(void ** instance, unsigned int attachID,
                              const BPElement * contextMap);
    
/**
 * Destroy a service instance allocated with BPPAllocate.
 * 
 *  \param instance An instance pointer returned from a BPPAllocate call.
 */
typedef void (*BPPDestroyPtr)(void * instance);    

/**
 *  Invoke a service method.
 *
 *  \param instance An instance pointer returned from a BPPAllocate call.
 *  \param functionName The name of the function being invoked
 *  \param tid The transaction id of this function invocation.  Should
 *             be passed by the service to BPCPostResultsFuncPtr
 *             or BPCPostErrorFuncPtr
 *  \param arguments The validated arguments to the function.  The service
 *                   is guaranteed that all defined arguments to the function
 *                   from the function description structure have been
 *                   checked, and that no unsupported arguments are present,
 *                   nor are required arguments missing.  This is always a
 *                   BPTMap.
 */
typedef void (*BPPInvokePtr)(void * instance, const char * functionName,
                             unsigned int tid, const BPElement * arguments);

/**
 * The "attach" function supports interpreter services.  These are services
 * which can be used by other services.  The primary types of services
 * that will be interested in this functionality are high level language
 * interpretation services which allow the authoring of services in
 * non-compiled languages.
 *
 * For most services Attach and Detach may be NULL which indicates that the
 * service may not be "used" by other services.
 *
 * Attach is called after BPPInitialize at the time the dependant service
 * is itself initialize.  Multiple dependant services may use the same
 * provider service, and the provider service may also expose functions
 * directly.  Multiple attached dependant services may be disambiguated
 * using the "attachID".  At the time an instance of a attached service
 * is instantiated, the attachID is passed in.
 *
 * The parameters map contains a set of parameters which describe the
 * dependant service.  These parameters are both set by BrowserPlus, and
 * extracted from the manifest of the dependant service.  
 *
 * The returned service definition describes the interface of the
 * dependent service.  This will likely be dynamically allocated
 * memory, which should not be freed until detach is called
 *
 * \warning this is an exception to the ServiceAPI's memory contract,
 *          and will be fixed in a later version of the ServiceAPI
 */
typedef const BPCoreletDefinition * (*BPPAttachPtr)(
    unsigned int attachID, const BPElement * parameterMap);

/**
 * At the time the last instance of a dependant service is deleted, detach
 * is called.  
 */
typedef void (*BPPDetachPtr)(unsigned int attachID);

//#define BPP_CORELET_API_VERSION 4

typedef struct BPPFunctionTable_t 
{
    /** The version of the service API to which this service plugin is
     *  written (use the BPP_CORELET_API_VERSION macro!) */ 
    unsigned int coreletAPIVersion;
    BPPInitializePtr initializeFunc;
    BPPShutdownPtr shutdownFunc;
    BPPAllocatePtr allocateFunc;
    BPPDestroyPtr destroyFunc;
    BPPInvokePtr invokeFunc;
    BPPAttachPtr attachFunc;
    BPPDetachPtr detachFunc;    
} BPPFunctionTable;

};

#endif
