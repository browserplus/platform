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
 * Written by Lloyd Hilaiel, on or around Fri May 18 17:06:54 MDT 2007 
 *
 * Suggested error code strings to use for common error cases.
 * by using these you increase the symmetry between your services
 * and others.
 */

#ifndef __BPERROR_H__
#define __BPERROR_H__

#ifdef __cplusplus
extern "C" {
#endif    


/** Invalid Parameters.  BrowserPlus validates the types of top
 *  level parameters, but cannot validate semantic meaning
 *  (a string as enum), nor complex parameters (Maps and Lists) */
#define BPE_INVALID_PARAMETERS "invalidParameters"
/** An error code to return when functionality is not yet implemented */
#define BPE_NOT_IMPLEMENTED "notImplemented"
/** the service encounterd an internal error which prevented the
 *  completion of the function execution.  Be more specific when
 *  possible */
#define BPE_INTERNAL_ERROR "internalError"

#ifdef __cplusplus
};
#endif    

#endif
