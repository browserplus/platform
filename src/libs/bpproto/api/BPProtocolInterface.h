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
 * (c) Yahoo 2007, all rights reserved
 * Written by Lloyd Hilaiel, on or around Tue May  8 15:11:53 MDT 2007
 *
 * This is the interface to a library which encapsulates the lightweight
 * protocol spoken by BPCore.
 *
 */

#include <ServiceAPI/bptypes.h>
#include <ServiceAPI/bpdefinition.h>

#ifndef __BPPROTOCOLINTERFACE_H__
#define __BPPROTOCOLINTERFACE_H__
#ifdef __cplusplus
extern "C" {
#endif    

#define BP_PROTO_MAX_SERVICES 32

    /** a handle to an allocated BPProtocol Instance */
    typedef struct BPProto_t * BPProtoHand;

    /** error codes returned by this library */
    typedef enum
    {
        BP_EC_OK = 0,
        /** a unspecified internal error has occured */
        BP_EC_GENERIC_ERROR = 1,
        BP_EC_CONNECTION_FAILURE = 2,
        /** the location argument to BPConnect is invalid */
        BP_EC_INVALID_LOCATION = 3,
        /** one of the paramters was invalid */
        BP_EC_INVALID_PARAMETER = 4,
        /** invalid interface usage.  You must connect a
         *  ProtocolHandle before invoking certain functions */
        BP_EC_INVALID_STATE = 5,
        /** The server returned a unsuccessful status code for a request */
        BP_EC_PROTOCOL_ERROR = 7,
        /** The feature is not yet implemented */
        BP_EC_NOT_IMPLEMENTED = 8,
        /** the call referred to a service that does not exist */
        BP_EC_NO_SUCH_CORELET = 9,
        /** the call to a function that doesn't exist on the specified
         *  service */
        BP_EC_NO_SUCH_FUNCTION = 10,
        /** the _service_ experienced an error while servicing your
         *  request */
        BP_EC_CORELET_EXEC_ERROR = 11,
        /** The version string is invalid (should by X.Y.Z) */
        BP_EC_INVALID_VERSION_STRING = 12,
        /** An "extended error" is an error that is opaque to the protocol
         *  library.  The return value should be a map which includes
         *  include an "error" key which is a unique string that may be
         *  used for localization, and may include a "verboseError",
         *  which is an english string that may be of use to developers. */
        BP_EC_EXTENDED_ERROR = 14,
        /** during a BPConnect, we failed to spawn the browserplus daemon */
        BP_EC_SPAWN_FAILED = 15,
        /** during a BPConnect, we were unable to connect to the daemon
         *  in the allowed amount of time */
        BP_EC_CONNECT_TIMEOUT = 16,
        /** unable to get updated permissions */
        BP_EC_PERMISSIONS_ERROR = 17,
        /** platform is blacklisted */
        BP_EC_PLATFORM_BLACKLISTED = 18,
        /** domain blacklist check failed */
        BP_EC_UNAPPROVED_DOMAIN = 19,
        /** BrowserPlus is not installed */
        BP_EC_NOT_INSTALLED = 20,
        /** service require failed */
        BP_EC_REQUIRE_ERROR = 21,
        /** The browserplus platform has been disabled by the user */
        BP_EC_PLATFORM_DISABLED = 22,
        /** The connection was asynchronously ended by the server
         *  (BrowserPlusCore) */
        BP_EC_PEER_ENDED_CONNECTION = 23,
        /** This platform has been uninstalled, and a newer version has
         *  been detected on disk */
        BP_EC_SWITCH_VERSION = 24
    } BPErrorCode;
        
    /** 
     * A debugging return which will return a C string containing english
     * text which describes an error code
     */
    const char * BPErrorCodeToString(BPErrorCode ec);

    /**
     * Initialize the protocol library.  should be called once at
     * process startup or library load.
     * This should be the first function called.
     */
    void BPInitialize();

    /**
     * Shutdown the protocol library.  should be called once at
     * process shutdown or library unload.  All allocated handles should
     * be freed before calling shutdown.
     */
    void BPShutdown();

    /**
     * allocate a protocol handle.
     */
    BPProtoHand BPAlloc();

    /**
     * free a protocol handle and all associated resources
     */
    void BPFree(BPProtoHand hand);

    /**    
     * A callback to return the status of a BPConnect call. 
     * \param ec the error code resulting from the connect call
     * \param cookie the same value passed into BPConnect
     */
    typedef void (*BPConnectCallback)(BPErrorCode ec, 
                                      void * cookie,
                                      const char * error,
                                      const char * verboseError);

    /**
     * asynchronously connect to BPCore, 
     *
     * \param hand A valid protocol handle allocated with BPAlloc()
     * \param uri A uri which describes the client for this session
     *            usually the full URL to the webpage that is initiating
     *            the connection to BrowserPlusCore.  For native clients
     *            we must invent a method.
     * \param locale The locale of the end user
     * \param userAgent An optional Informational string identifying the
     *                  program which is connecting to BrowserPlusCore
     */
    BPErrorCode BPConnect(BPProtoHand hand,
                          const char * uri,
                          const char * locale,
                          const char * userAgent,
                          BPConnectCallback connectCB,
                          void * cookie);

    /**    
     * A callback invoked each time the user must be prompted.
     * a response should be delivered by calling BPDeliverUserResponse()
     *
     * \param cookie the same value passed into BPSetUserPromptCallback
     * \param pathToHTMLDialog the path to the HTML holding the user prompt
     * \param arguments arguments that should be mapped into javascript and
     *                  passed to the dialog.
     * \param tid a integer that identifies this query and should be
     *            passed into BPDeliverUserResponse()
     */
    typedef void (*BPUserPromptCallback)(void * cookie,
                                         const char * pathToHTMLDialog,
                                         const BPElement * arguments,
                                         unsigned int tid);

    /**
     * Set a callback handler on a session to handle incoming requests
     * to prompt the user.  This should be called after BPAllocate and
     * before BPConnect.  Failure to set a UserPromptCallback will result
     * in all requests to the user to be denied.
     * The callback will not be invoked after BPFree() returns.
     */
    BPErrorCode BPSetUserPromptCallback(BPProtoHand hand,
                                        BPUserPromptCallback cb,
                                        void * cookie);
    
    /**    
     * Deliver a response to a user prompt request.
     *
     * \param cookie the same value passed into BPSetUserPromptCallback
     * \param message the user prompt to display
     * \param tid a integer that identifies this query and should be
     *            passed into BPDeliverUserResponse()
     * \param userResponse zero is false, nonzero is true
     */
    BPErrorCode BPDeliverUserResponse(BPProtoHand hand,
                                      unsigned int tid,
                                      const BPElement * response);

    /**    
     * A callback to return the description of a service.
     * \param ec The status of the request.  If ec == BP_EC_EXTENDED_ERROR,
     *           results will contain the extended error information.
     * \param cookie the same value passed into BPDescribe
     * \param def A structure containing the requested definition of the
     *            service.  This is typed and tra
     * \param error for BP_EC_EXTENDED_ERROR, the error code
     * \param error for BP_EC_EXTENDED_ERROR, optional verbose error string
     */
    typedef void (*BPDescribeCallback)(BPErrorCode ec,
                                       void * cookie,
                                       const BPCoreletDefinition * def,
                                       const char * error,
                                       const char * verboseError);

    /**
     * describe the interface of a service.  Note that the functionPointer
     * element of the BPFunctionDefinition structure will be NULL, and
     * BPExecute must be used to execute functions on services
     */
    BPErrorCode BPDescribe(BPProtoHand hand,
                           const char * serviceName,
                           const char * serviceVersion,
                           const char * serviceMinversion,
                           BPDescribeCallback describeCB,
                           void * cookie);
    
    /**    
     * A callback for the invocation of callback parameters.  These are
     * callbacks that are passed into the 
     * \param cookie the same value passed into BPExecuteASync
     * \param tid the transaction id to which results have been recieved
     * \param ec error code.
     * \param results - the data to be passed to the callback
     */
    typedef void (*BPInvokeCallback)(void * cookie,
                                     unsigned int tid,
                                     long long int callbackHandle,
                                     const BPElement * results);
    
    /**    
     * A callback to return the description of required services.
     * \param ec The status of the request.  if not BP_EC_OK,
     *           def will be NULL.
     * \param cookie the same value passed into BPRequire
     * \param defs An array of pointers to structures containing 
     *             the definitions of the requested services
     * \param numDefs the number of elements in defs
     * \param error If ec == BP_EC_EXTENDED_ERROR, a string representation
     *              of the error returned.
     * \param verboseError If ec == BP_EC_EXTENDED_ERROR, this may be
     *              NULL or a string containing a human readable description
     *              of the error encountered.   
     */
    typedef void (*BPRequireCallback)(BPErrorCode ec,
                                      void * cookie,
                                      const BPCoreletDefinition ** defs,
                                      unsigned int numDefs,
                                      const char * error,
                                      const char * verboseError);

    /**
     * Require services (may cause them to be installed).
     * \param services - a BPList of BPMaps describing each service.
     *                   Each map must have key "name", "version" and
     *                   "minversion" keys are optional.  The services
     *                   specified will be installed, or an error will
     *                   be returned in 'ec' of require callback
     */
    BPErrorCode BPRequire(BPProtoHand hand,
                          const BPElement * args,
                          BPRequireCallback requireCB,
                          void * requireCookie,
                          BPInvokeCallback invokeCB,
                          void * invokeCookie,
                          unsigned int * tid);

    /**    
     *  A generic callback signature.  Used by multiple functions.
     *  The type of "value" is documented with the function that takes
     *  a BPGenericCallback as an argument.
     */
    typedef void (*BPGenericCallback)(BPErrorCode ec,
                                      void * cookie,
                                      const BPElement * value);

    /**
     * Enumerate the available services.
     * 
     * The value passed to BPGenericCallback will be a list of maps of
     * available services, each containing 'name' and 'version' keys, both
     * which will have a string value.
     */
    BPErrorCode BPEnumerate(BPProtoHand hand,
                            BPGenericCallback enumerateCB,
                            void * cookie);

    /**
     * Instantiate an instance of a service.
     *
     * The 'value' passed to BPGenericCallback will be a BPTInteger,
     * containing a numeric id of the allocated instance.
     *
     * TODO: not yet implemented
     */
    BPErrorCode BPInstantiate(BPProtoHand hand,
                              const char * serviceName,
                              const char * serviceVersion,
                              BPGenericCallback enumerateCB,
                              void * cookie);

    /**
     * Release an instance of a service allocated with BPInstantiate.
     *
     * TODO: not yet implemented
     */
    BPErrorCode BPRelease(BPProtoHand hand,
                          long long instanceID);

    /**    
     * A callback for the return of results
     * \param cookie the same value passed into BPExecute*
     * \param tid the transaction id to which results have been recieved
     * \param callbackHandle the handle of the callback to invoke,
     *                       which was passed as a parameter to the
     *                       BPExecute* function
     * \param results - the return value of the service function
     *                  invocation.
     */
    typedef void (*BPResultsCallback)(void * cookie,
                                      unsigned int tid,
                                      BPErrorCode ec,
                                      const BPElement * results);

    /**
     * Asynchronously execute a query
     *
     * \param invokeCB a callback that will be called when callaback
     *        parameters are invoked
     * \param invokeCookie a void pointer that will be held and passed to your
     *        invoke callback
     * \param resultsCB a callback that will be called when results are
     *        available
     * \param resultsCookie a void pointer that will be held and passed to your
     *        results callback
     * \param tid a 32 bit transaction id that will be passed to the
     *            callback.  This is a mechanism that the client may
     *            use to 
     *
     * \warning The callback will be called on a separate thread.
     *          The client is resposibile for synchronization.
     *        
     * The callback will always be called, unless the transaction is
     * cancelled, or the connection handle is deallocated before re
     */  
    BPErrorCode BPExecute(BPProtoHand hand,
                          const char * service,
                          const char * serviceVersion,
                          const char * functionName,
                          const BPElement * args,
                          BPResultsCallback resultsCB,
                          void * resultsCookie,
                          BPInvokeCallback invokeCB,
                          void * invokeCookie,
                          unsigned int * tid);

    /** 
     * Cancel a running transaction.  This call will cancel a running
     * transaction when possible.  It is gauranteed that at the point the
     * function returns, if BPResultsCallback has not been called, it
     * will not be invoked.  Note, however, that because results are returned
     * upon a different thread, that the callback will be called while
     * BPCancel is being invoked.  Clients should take necessary
     * precautions.
     *
     * \param hand A valid protocol handle
     * \param tid the transaction id returned from BPExecute to be canceled.
     */
    BPErrorCode BPCancel(BPProtoHand hand, unsigned int tid);

    BPErrorCode BPGetState(BPProtoHand hand,
                           const char * state,
                           BPGenericCallback stateCB,
                           void * cookie);

    BPErrorCode BPSetState(BPProtoHand hand,
                           const char * state,
                           const BPElement * newValue);


#ifdef __cplusplus
};
#endif    
#endif
