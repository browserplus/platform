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

#ifndef __BPPFUNCTIONS_H__
#define __BPPFUNCTIONS_H__

#include <ServiceAPI/bptypes.h>
#include <ServiceAPI/bpdefinition.h>
#include <ServiceAPI/bpcfunctions.h>

#ifdef __cplusplus
extern "C" {
#endif    

/**
 * Initialize the service, called once at service load time.
 *
 * 
 * \param coreFunctionTable  pointer to a structure of pointers to BPC
 *                           functions that the service may call through into
 *                           BPCore.
 * \param serviceDir         The directory in which the service being initialized
 *                           is installed.
 * \param dependentDir       In the case of a provider service, the path to the 
 *                           dependent service being loaded.
 * \param dependentParams    In the case of a provider service, arguments from
 *                           the manifest.json of the dependent service.
 *                 
 * \return  A service definition which describes the interface of the
 *          service.  This memory should not be freed until the service
 *          is shutdown.
 */
typedef const BPServiceDefinition * (*BPPInitializePtr)(
    const BPCFunctionTable * coreFunctionTable,
    const BPPath serviceDir,
    const BPPath dependentDir,
    const BPElement * dependentParams);

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
 * \param uri   a UTF8 encoded string containing a URI of the current client. This is
 *              usually the full URL to the webpage that is interacting
 *              with browserplus.  In the case of a native local application
 *              interacting with browserplus it should be a URI with a
 *              method of 'bpclient' (i.e. bpclient://CLIENTIDENTIFIER').
 * \param serviceDir is an absolute path to the directory containing the
 *              files distributed with the service.
 * \param dataDir is an absolute path to where the service should store
 *                any data that needs to persist.
 * \param tempDir is the path to a directory that may be used for
 *                temporary data.  it will be unique for every instance, and will
 *                be purged when the instance is deallocated.
 * \param locale The locale of the end user to which strings, if any, should be localized.
 * \param userAgent The client user-agent as a UTF8 encoded string.
 * \param clientPid The process ID of the client program/browser.
 *
 * \return zero on success, non-zero on failure
 */
typedef int (*BPPAllocatePtr)(
    void ** instance, const BPString uri, const BPPath serviceDir,
    const BPPath dataDir, const BPPath tempDir, const BPString locale,,
    const BPString userAgent, int clientPid);
    
/**
 * Destroy a service instance allocated with BPPAllocate.
 * 
 *  \param instance An instance pointer returned from a BPPAllocate call.
 */
typedef void (*BPPDestroyPtr)(void * instance);    

/**
 *  Invoke a service method.
 *
 *  \param instance The instance pointer returned from a BPPAllocate call.
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
 *  Cancel a transaction previously started by calling BPPInvokePtr()
 *
 *  \param instance The instance pointer returned from a BPPAllocate call.
 *  \param tid The transaction id of this function invocation.
 */
typedef void (*BPPCancelPtr)(void * instance, unsigned int tid);

/**
 * A callback invoked exactly once immediately after a service is installed
 * on disk.  This is an opportunity for a service to perform any one-time setup.
 *
 * \param serviceDir is an absolute path to the directory containing the
 *              files distributed with the service.
 * \param dataDir is an absolute path to where the service should store
 *                any data that needs to persist.
 */
typedef int (*BPPInstallPtr)(const BPPath serviceDir, const BPPath dataDir);

/**
 * A callback invoked exactly once immediately before a service is purged from
 * disk.  This is an opportunity for a service to perform any one-time cleanup.
 *
 * \param serviceDir is an absolute path to the directory containing the
 *              files distributed with the service.
 * \param dataDir is an absolute path to where the service should store
 *                any data that needs to persist.
 */
typedef int (*BPPUninstallPtr)(const BPPath serviceDir, const BPPath dataDir);


#define BPP_SERVICE_API_VERSION 5

typedef struct BPPFunctionTable_t 
{
    /** The version of the service API to which this service is
     *  written (use the BPP_SERVICE_API_VERSION macro!) */ 
    unsigned int serviceAPIVersion;
    BPPInitializePtr initializeFunc;
    BPPShutdownPtr shutdownFunc;
    BPPAllocatePtr allocateFunc;
    BPPDestroyPtr destroyFunc;
    BPPInvokePtr invokeFunc;
    BPPCancelPtr cancelFunc;
    BPPInstallPtr installFunc;
    BPPUninstallPtr installFunc;
} BPPFunctionTable;

/**
 * The single entry point into the plugin which attains a
 * BPPFunctionTable containing the version.  Having a single symbol 
 * which is sought in the plugin interface allows the service author to
 * strip all other symbols.
 */
#ifdef WIN32
__declspec(dllexport)
#endif
const BPPFunctionTable * BPPGetEntryPoints(void);

#ifdef __cplusplus
};
#endif    

#endif
