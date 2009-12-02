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
 *  bpregistry.h
 *  Registry utilities.
 *
 *  Created by David Grigsby on 4/15/09.
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef __BPREGISTRY_H__
#define __BPREGISTRY_H__

#include <string>
#include <vector>
// TODO: it's sub-optimal to include all of windows.h just for HKEY.
#include <windows.h>



////////////////////////////////////////
// Unimplemented Use Cases:
//
//  3)  add/remove a subkey
//  4)  add/remove a value


////////////////////
// Test Cases:
//
//  1)  read a value that does/doesn't exist
//  2)  opening/operating on a top-level root key
//  3)  write a value to key that doesn't exist


namespace bp {
namespace registry {

// Forward Declarations
class Key;
class Value;


//////////////////////////////
// Convenience Methods
//
// Note: these methods may throw bp::Exception.
//

// Create the specified key.  No-op if the key already exists.
// ex: createKey( "HKLM\\Software\\Company" )
void createKey( const std::string& sKey );

// Starting at specified key, do a deep search for specified values.
// Append the data for matching values to the results vector.
void deepFindData( const Key& key, const std::string& sFindValueName,
                   std::vector<std::string>& vsResults );

// Starting at specified key, do a deep search for specified values.
// Append the data for matching values to the results vector.
// This is a convenience overload of method above.
// The const char* argument type for cszKey is intentional.
void deepFindData( const char* cszKey, const std::string& sFindValueName,
                   std::vector<std::string>& vsResults );

// Delete the specified key.
void deleteKey( const std::string& sKey );

bool keyExists( const std::string& sKey );

// Read integer data from the default value of the specified key.
int readInt( const std::string& sKey );

// Read integer data from the specified value of the specified key.
int readInt( const std::string& sKey,
             const std::string& sValueName );

// Read string data from the default value of the specified key.
std::string readString( const std::string& sKey );

// Read string data from the specified value of the specified key.
std::string readString( const std::string& sKey,
                        const std::string& sValueName );

// Return all subkeys of the specified key.
std::vector<Key> subKeys( const std::string& sKey );

// Write integer data to the default value of the specified key.
void writeInt( const std::string& sKey,
               int nData );

// Write integer data to the specified value of the specified key.
void writeInt( const std::string& sKey,
               const std::string& sValueName,
               int nData );

// Write string data to the default value of the specified key.
void writeString( const std::string& sKey,
                  const std::string& sData );

// Write string data to the specified value of the specified key.
void writeString( const std::string& sKey,
                  const std::string& sValueName,
                  const std::string& sData );


                        
//////////////////////////////
// Class bp::registry::Key
//
// Note: methods of this class may throw bp::Exception.
//
class Key
{
public:
    // Default copy ctor/op= are ok.

    // ex: Key( "HKLM\\Software\\Company" )
    Key( const std::string& sFullPath );

    void        create() const;

    bool        exists() const;
    
    std::string fullPath() const;

    int         numSubKeys() const;
    
    int         numValues() const;
    
    std::string path() const;

    // Read integer data from the default value of the key.
    int         readInt() const;

    // Read integer data from the specified value of the key.
    int         readInt( const std::string& sValueName ) const;

    // Read string data from the default value of the key.
    std::string readString() const;

    // Read string data from the specified value of the key.
    std::string readString( const std::string& sValueName ) const;

    void        remove() const;

    std::vector<Key> subKeys() const;

    Key         subKey( int nIdx ) const;

    Value       value( int nIdx ) const;
    
    // Write integer data to the default value of the key.
    void        writeInt( int nData );

    // Write integer data to the specified value of the key.
    void        writeInt( const std::string& sValueName,
                          int nData );
    
    // Write string data to the default value of the key.
    void        writeString( const std::string& sData );

    // Write string data to the specified value of the key.
    void        writeString( const std::string& sValueName,
                             const std::string& sData );
    
private:
    HKEY         m_hRootKey;
    std::wstring m_wsPath;
};


class Value
{
public:    
    // Default copy ctor/op= are ok.
    Value( const std::string& sName, const Key* pParentKey, DWORD dwValueType );

    std::string name() const;
    DWORD       type() const;
    
    std::string readString() const;

private:
    std::wstring m_wsName;
    const Key*   m_pParentKey;
    DWORD        m_dwValueType;
};



} // registry
} // bp


#endif // __BPREGISTRY_H__

