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
 * Written by Lloyd Hilaiel, on or around Fri May 18 17:06:54 MDT 2007 
 *
 * BPC* functions are provided by BPCore and called by the corelet 
 */

#ifndef __BPCFUNCTIONS_H__
#define __BPCFUNCTIONS_H__

#include <ServiceAPI/bptypes.h>
#include <ServiceAPI/bpdefinition.h>

#ifdef __cplusplus
extern "C" {
#endif    

/**
 * Post results from a called BPPFunctionPtr.
 * Subsequent to this call, no callbacks passed as parameters to the
 * service function may be called.  This indicates the completion of the
 * function invocation or transaction.
 *
 * \param tid a transaction id passed into the service via the invocation
 *        of a BPPFunctionPtr
 * \param results The results of the function invocation.
 */
typedef void (*BPCPostResultsFuncPtr)(unsigned int tid,
                                      const BPElement * results);

/**
 * Post an asynchronous error from a called BPPFunctionPtr.
 * Subsequent to this call, no callbacks passed as parameters to the
 * service function may be called.  This indicates the completion of the
 * function invocation or transaction.
 *
 * \param tid a transaction id passed into the corelet via the invocation
 *        of a BPPFunctionPtr
 * \param error A string representation of an error.  If NULL, a generic
 *              error will be raised.  These strings should be camel
 *              cased english phrases.  The service name will be prepended
 *              to the error, and it will be propogated up to the
 *              caller.    
 * \param verboseError An optional english string describing more verbosely
 *                     exactly what went wrong.  This is intended for
 *                     developers.
 */
typedef void (*BPCPostErrorFuncPtr)(unsigned int tid,
                                    const char * error,
                                    const char * verboseError);

/**
 * Invoke a callback that was passed into a service function as a
 * parameter.
 *
 * \param tid a transaction id passed into the service via the invocation
 *        of a BPPFunctionPtr
 * \param callbackHandle A callback handle attained from the original
 *        function invocation. 
 * \param params - the argument to the callback.  
 */
typedef void (*BPCInvokeCallBackFuncPtr)(unsigned int tid,
                                         BPCallBack callbackHandle,
                                         const BPElement * params);

/** log level argument to BPCLogFuncPtr, debug message*/
#define BP_DEBUG 1
/** log level argument to BPCLogFuncPtr, informational message*/
#define BP_INFO 2
/** log level argument to BPCLogFuncPtr, warning message*/
#define BP_WARN 3
/** log level argument to BPCLogFuncPtr, error message*/
#define BP_ERROR 4
/** log level argument to BPCLogFuncPtr, fatal message*/
#define BP_FATAL 5
/**
 * Output a log a message. 
 * \param level the severity of the log message
 * \param fmt a printf() style format string
 * \param varags arguments to the printf() style format string
 */
typedef void (*BPCLogFuncPtr)(unsigned int level, const char * fmt, ...);

/**
 * The signature of a function capable of recieving a response to a
 * promptUser request.
 *
 * \param context the same void * pointer passed to the BPCPromptUserFuncPtr
 * \param promptId the same unsigned int returned from BPCPromptUserFuncPtr
 * \param response the user's response, a data structure mapped from
 *          javascript.
 */
typedef void (*BPUserResponseCallbackFuncPtr)(
    void * context,
    unsigned int promptId,
    const BPElement * response);

/**
 * Prompt the end user.  end user prompting must be associated with
 * a specific transaction/method invocation.  
 *
 * \param tid a transaction id passed into the service via the invocation
 *        of a BPPFunctionPtr
 * \param utf8PathToHTMLDialog A utf8 string holding the absolute path
 *          to the dialog you wish to display.  
 * \param arguments The arguments to make available to the dialog
 *          via the BPDialog.args() call
 * \param responseCallback A Function pointer to invoke when the response
 *          to the user prompt is available.
 * \param context a pointer to pass to the responseCallback 
 *
 * \returns A unsigned prompt ID that wil be passed to the
 *          responseCallback
 */
typedef unsigned int (*BPCPromptUserFuncPtr)(
    unsigned int tid,
    const char * utf8PathToHTMLDialog,
    const BPElement * arguments,
    BPUserResponseCallbackFuncPtr responseCallback,
    void * context);

/**
 * A table containing function pointers for functions available to be
 * called by services implemented by BrowserPlus.
 */
typedef struct {
    BPCPostResultsFuncPtr postResults;
    BPCPostErrorFuncPtr postError;
    BPCLogFuncPtr log;
    BPCInvokeCallBackFuncPtr invoke;
    BPCPromptUserFuncPtr prompt;
} BPCFunctionTable;

#ifdef __cplusplus
};
#endif    

#endif
