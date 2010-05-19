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
 * Written by Lloyd Hilaiel, on or around Fri May 18 17:06:54 MDT 2007 
 *
 * Types.  These basic types are how arguments are passed to services,
 * and how responses are returned from services.  The representation
 * allows introspection which allows data to be serialized or bound to
 * native types in different target execution environments.
 */

#ifndef __BPTYPES_H__
#define __BPTYPES_H__

#ifdef __cplusplus
extern "C" {
#endif    

/**
 *  The available types that may be passed across the BrowserPlus <-> Service
 *  boundary.
 */
typedef enum {
    BPTNull,    /*!< Null type */
    BPTBoolean, /*!< Boolean type */
    BPTInteger, /*!< integer type */       
    BPTDouble,  /*!< floating point type */
    BPTString,  /*!< string type, all strings are represented in UTF8 */ 
    BPTMap,     /*!< map (hash) type. */
    BPTList,    /*!< list (array) type. */
    BPTCallBack,/*!< callback type. */
    BPTPath,    /*!< pathname type - represented as a string however
                     paths are sensitive and are handled differently
                     than strings */
    BPTAny      /*!< When specified in an argument description, denotes
                     that any data type is allowable. */
} BPType;

/* definition of basic types */

/** booleans are represented as integers */
typedef int BPBool;
/** BPBool true value */
#define BP_TRUE 1
/** BPBool false value */
#define BP_FALSE 0
/** strings are UTF8 */
typedef char * BPString;
/** integers are 64 bit signed entities */
typedef long long int BPInteger;        
/** floating point values are double precision */
typedef double BPDouble;        

/** the callback is an integer handle */
typedef BPInteger BPCallBack;        

/** A structure representing of a list */
typedef struct {
    /** The number of elements in the list */
    unsigned int size;
    /** an array of pointers to child elements */
    struct BPElement_t ** elements;
} BPList;

/** a structure representing a map */
typedef struct {
    /** The number of elements in the list */
    unsigned int size;
    /** An array of BPMapElem structures containing keys and values */
    struct BPMapElem_t * elements;
} BPMap;

/** pathnames are UTF8 */
typedef char * BPPath;

/** The uber structure capable of representing any element */
typedef struct BPElement_t {
    /** The type of element that this structure contains.  This type
     *  determine which union value is valid */
    BPType type;
    /** The value of the element */
    union 
    {
        BPBool booleanVal;
        BPString stringVal;
        BPInteger integerVal;        
        BPDouble doubleVal;        
        BPMap mapVal;
        BPList listVal;
        BPCallBack callbackVal;
        BPPath pathVal;
    } value;
} BPElement;

/** A BPMapElem contains a pointer to a UTF8 string, and a pointer to
 *  a BPElement structure containing the value */
typedef struct BPMapElem_t {
    BPString key;        
    BPElement * value;
} BPMapElem;

#ifdef __cplusplus
};
#endif    

#endif
