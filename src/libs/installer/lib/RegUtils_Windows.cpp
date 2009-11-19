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


#include "RegUtils_Windows.h"
#include "BPUtils/bpconvert.h"
#include "BPUtils/bperrorutil.h"
#include "BPUtils/bpstrutil.h"


using namespace std;
using namespace bp::strutil;


namespace bp {
    namespace registry {


        ////////////////////
        // Free Functions
        //

        void createKey( const std::string& sKey )
        {
            Key key( sKey );
            if (!key.exists()) {
                key.create();
            }
        }


        void deepFindData( const Key& key, const std::string& sFindValueName,
                           std::vector<std::string>& vsResults )
        {
            // Search among our values.
            for (int i=0; i<key.numValues(); ++i) {
                Value val = key.value( i );
                if (val.name() == sFindValueName) {
                    vsResults.push_back( val.readString() );
                }
            }

            // Search our subkeys.
            for (int i=0; i<key.numSubKeys(); ++i) {
                deepFindData( key.subKey(i), sFindValueName, vsResults );
            }
        }


        void deepFindData( const char* cszKey, const std::string& sFindValueName,
                           std::vector<std::string>& vsResults )
        {
            deepFindData( Key(cszKey), sFindValueName, vsResults );
        }


        bool keyExists( const std::string& sKey )
        {
            return Key( sKey ).exists();
        }


        std::vector<Key> subKeys( const std::string& sKey )
        {
            return Key( sKey ).subKeys();
        }


        void deleteKey( const std::string& sKey )
        {
            Key key( sKey );
            if (key.exists()) {
                key.remove();
            }
        }


        int readInt( const std::string& sKey )
        {
            return Key( sKey ).readInt();
        }


        int readInt( const std::string& sKey,
                     const std::string& sValueName )
        {
            return Key( sKey ).readInt( sValueName );
        }


        std::string readString( const std::string& sKey )
        {
            return Key( sKey ).readString();
        }


        std::string readString( const std::string& sKey,
                                const std::string& sValueName )
        {
            return Key( sKey ).readString( sValueName );
        }


        void writeInt( const std::string& sKey,
                       int nData )
        {
            Key( sKey ).writeInt( nData );
        }


        void writeInt( const std::string& sKey,
                       const std::string& sValueName,
                       int nData )
        {
            Key( sKey ).writeInt( sValueName, nData );
        }


        void writeString( const std::string& sKey,
                          const std::string& sData )
        {
            Key( sKey ).writeString( sData );
        }


        void writeString( const std::string& sKey,
                          const std::string& sValueName,
                          const std::string& sData )
        {
            Key( sKey ).writeString( sValueName, sData );
        }


        static HKEY rootKeyFromString( const std::string& sIn )
        {
            string sName = toUpper( sIn );

            if ((sName == "HKEY_CLASSES_ROOT") || (sName == "HKCR"))
                return HKEY_CLASSES_ROOT;
            else if ((sName == "HKEY_CURRENT_USER") || (sName == "HKCU"))
                return HKEY_CURRENT_USER;
            else if ((sName == "HKEY_LOCAL_MACHINE") || (sName == "HKLM"))
                return HKEY_LOCAL_MACHINE;
            else if ((sName == "HKEY_USERS") || (sName == "HKU"))
                return HKEY_USERS;
            else
                BP_THROW( "RootKeyFromString found no match for: " + sIn );
        }


        static std::string stringFromRootKey( HKEY hKey )
        {
            if (hKey == HKEY_CLASSES_ROOT)
                return "HKCR";
            else if (hKey == HKEY_CURRENT_USER)
                return "HKCU";
            else if (hKey == HKEY_LOCAL_MACHINE)
                return "HKLM";
            else if (hKey == HKEY_USERS)
                return "HKU";
            else
                BP_THROW( "StringFromRootKey found no match for: " +
                          bp::conv::toString( hKey ) );
        }


        ////////////////////
        // Key Methods
        //

        Key::Key( const std::string& sPath ) :
            m_wsPath(),
            m_hRootKey()
        {
            // The first part of input path should be root key name.
            size_t nPos = sPath.find( "\\" );
            string sRootKey = sPath.substr( 0, nPos );
            m_hRootKey = rootKeyFromString( sRootKey );
    
            // We'll store our path as everything after the root key and separator.
            m_wsPath= utf8ToWide( sPath.substr( nPos+1 ) );
        }


        void Key::create() const
        {
            HKEY hKey;
            LONG lRtn = RegCreateKeyExW( m_hRootKey, m_wsPath.c_str(), 0, NULL, 0,
                                         KEY_SET_VALUE, NULL, &hKey, NULL );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegCreateKeyExW returned: " + bp::conv::toString( lRtn ) );
            }

            lRtn = RegCloseKey( hKey );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegCloseKey returned: " + bp::conv::toString( lRtn ) );
            }
        }


        bool Key::exists() const
        {
            HKEY hKey;
            LONG lRtn = RegOpenKeyExW( m_hRootKey, m_wsPath.c_str(),
                                       0, KEY_READ, &hKey );
            if (lRtn != ERROR_SUCCESS) {
                return false;
            }
    
            lRtn = RegCloseKey( hKey );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegCloseKey returned: " + bp::conv::toString( lRtn ) );
            }

            return true;
        }


        int Key::numSubKeys() const
        {
            HKEY hKey;
            LONG lRtn = RegOpenKeyExW( m_hRootKey, m_wsPath.c_str(),
                                       0, KEY_QUERY_VALUE, &hKey );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegOpenKeyExW returned: " + bp::conv::toString( lRtn ) );
            }

            DWORD dwNumSubKeys;
            lRtn = RegQueryInfoKeyW( hKey, NULL, NULL, NULL,
                                     &dwNumSubKeys,
                                     NULL, NULL, NULL, NULL, NULL, NULL, NULL );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegQueryInfoKeyW returned: " + bp::conv::toString( lRtn ) );
            }

            return static_cast<int>( dwNumSubKeys );
        }


        int Key::numValues() const
        {
            HKEY hKey;
            LONG lRtn = RegOpenKeyExW( m_hRootKey, m_wsPath.c_str(),
                                       0, KEY_QUERY_VALUE, &hKey );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegOpenKeyExW returned: " + bp::conv::toString( lRtn ) );
            }

            DWORD dwNumValues;
            lRtn = RegQueryInfoKeyW( hKey, NULL, NULL, NULL, NULL, NULL, NULL, 
                                     &dwNumValues,
                                     NULL, NULL, NULL, NULL );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegQueryInfoKeyW returned: " + bp::conv::toString( lRtn ) );
            }

            return static_cast<int>( dwNumValues );
        }


        std::string Key::fullPath() const
        {
            return stringFromRootKey( m_hRootKey ) + "\\" + wideToUtf8( m_wsPath );
        }


        std::string Key::path() const
        {
            return wideToUtf8( m_wsPath );
        }


        int Key::readInt() const
        {
            // Specifying "" for value name tells win32 to use default value.
            return readInt( "" );
        }


        int Key::readInt( const std::string& sValueName ) const
        {
            HKEY hKey;
            LONG lRtn = RegOpenKeyExW( m_hRootKey, m_wsPath.c_str(),
                                       0, KEY_READ, &hKey );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegOpenKeyExW returned: " + bp::conv::toString( lRtn ) );
            }

            // TODO: could use RegQueryInfo to get required buf size.
            const int knBufSize = 2000;
            BYTE buf[knBufSize];
            DWORD dwBufSize = knBufSize;
            std::wstring wValueName = utf8ToWide( sValueName );
            lRtn = RegQueryValueExW( hKey, wValueName.c_str(),
                                     NULL, NULL, buf, &dwBufSize );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegQueryValueExW returned: " + bp::conv::toString( lRtn ) );
            }

            lRtn = RegCloseKey( hKey );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegCloseKey returned: " + bp::conv::toString( lRtn ) );
            }

            int* pnVal = reinterpret_cast<int*>( buf );
            return *pnVal;
        }


        std::string Key::readString() const
        {
            // Specifying "" for value name tells win32 to use default value.
            return readString( "" );
        }


        std::string Key::readString( const std::string& sValueName ) const
        {
            HKEY hKey;
            LONG lRtn = RegOpenKeyExW( m_hRootKey, m_wsPath.c_str(),
                                       0, KEY_READ, &hKey );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegOpenKeyExW returned: " + bp::conv::toString( lRtn ) );
            }

            // TODO: could use RegQueryInfo to get required buf size.
            const int knBufSize = 2000;
            BYTE szBuf[knBufSize];
            DWORD dwBufSize = knBufSize;
            std::wstring wValueName = utf8ToWide( sValueName );
            lRtn = RegQueryValueExW( hKey, wValueName.c_str(),
                                     NULL, NULL, szBuf, &dwBufSize );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegQueryValueExW returned: " + bp::conv::toString( lRtn ) );
            }

            lRtn = RegCloseKey( hKey );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegCloseKey returned: " + bp::conv::toString( lRtn ) );
            }
            std::wstring wval(reinterpret_cast<wchar_t*>( szBuf ) );
            std::string rval = wideToUtf8( wval );
            return rval;
        }


        void Key::remove() const
        {
            LONG lRtn = RegDeleteKeyW( m_hRootKey, m_wsPath.c_str() );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegDeleteKeyW returned: " + bp::conv::toString( lRtn ) );
            }
        }


        std::vector<Key> Key::subKeys() const
        {
            std::vector<Key> rval;
            int num = numSubKeys();
            for (int i = 0; i < num; i++) {
                rval.push_back( subKey( i ) );
            }
            return rval;
        }


        Key Key::subKey( int nIdx ) const
        {
            HKEY hKey;
            LONG lRtn = RegOpenKeyExW( m_hRootKey, m_wsPath.c_str(),
                                       0, KEY_READ, &hKey );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegOpenKeyExW returned: " + bp::conv::toString( lRtn ) );
            }

            // TODO: could use RegQueryInfo to get required buf size.
            const int knBufSize = 1000;
            wchar_t szSubKeyName[knBufSize];
            DWORD dwBufSize = knBufSize;
            lRtn = RegEnumKeyExW( hKey, nIdx, szSubKeyName, &dwBufSize,
                                  NULL, NULL, NULL, NULL );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegEnumKeyEx returned: " + bp::conv::toString( lRtn ) );
            }

            string sSubKeyFullPath = fullPath() + "\\" + wideToUtf8( szSubKeyName );
    
            return Key( sSubKeyFullPath );
        }


        Value Key::value( int nIdx ) const
        {
            HKEY hKey;
            LONG lRtn = RegOpenKeyExW( m_hRootKey, m_wsPath.c_str(),
                                       0, KEY_READ, &hKey );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegOpenKeyExW returned: " + bp::conv::toString( lRtn ) );
            }

            // TODO: could use RegQueryInfo to get required buf size.
            const int knBufSize = 1000;
            wchar_t szValName[knBufSize];
            DWORD dwBufSize = knBufSize;
            DWORD dwType;
            lRtn = RegEnumValueW( hKey, nIdx, szValName, &dwBufSize,
                                  NULL, &dwType, NULL, NULL );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegEnumValueW returned: " + bp::conv::toString( lRtn ) );
            }

            return Value( wideToUtf8( szValName ), this, dwType );
        }


        void Key::writeInt( int nData )
        {
            // Specifying "" for value name tells win32 to use default value.
            writeInt( "", nData );
        }


        void Key::writeInt( const std::string& sValueName,
                            int nData )
        {
            HKEY hKey;
            LONG lRtn = RegCreateKeyExW( m_hRootKey, m_wsPath.c_str(), 0, NULL, 0,
                                         KEY_SET_VALUE, NULL, &hKey, NULL );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegCreateKeyExW returned: " + bp::conv::toString( lRtn ) );
            }

            std::wstring wValueName = utf8ToWide( sValueName );
            lRtn = RegSetValueExW( hKey, wValueName.c_str(), 0, REG_DWORD, 
                                   reinterpret_cast<BYTE*>(&nData), sizeof(nData) );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegSetValueExW returned: " + bp::conv::toString( lRtn ) );
            }

            lRtn = RegCloseKey( hKey );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegCloseKey returned: " + bp::conv::toString( lRtn ) );
            }
        }


        void Key::writeString( const std::string& sData )
        {
            // Specifying "" for value name tells win32 to use default value.
            writeString( "", sData );
        }


        void Key::writeString( const std::string& sValueName,
                               const std::string& sData )
        {
            HKEY hKey;
            LONG lRtn = RegCreateKeyExW( m_hRootKey, m_wsPath.c_str(), 0, NULL, 0,
                                         KEY_SET_VALUE, NULL, &hKey, NULL );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegCreateKeyExW returned: " + bp::conv::toString( lRtn ) );
            }

            std::wstring wData = utf8ToWide( sData );
            std::wstring wValueName = utf8ToWide( sValueName );
            lRtn = RegSetValueExW( hKey, wValueName.c_str(), 0, REG_SZ, 
                                   reinterpret_cast<const BYTE*>(wData.c_str()),
                                   static_cast<DWORD>(wData.length() * sizeof(wchar_t)) );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegSetValueExW returned: " + bp::conv::toString( lRtn ) );
            }

            lRtn = RegCloseKey( hKey );
            if (lRtn != ERROR_SUCCESS) {
                BP_THROW( "RegCloseKey returned: " + bp::conv::toString( lRtn ) );
            }
        }


        ////////////////////
        // Value Methods
        //

        Value::Value( const std::string& sName,
                      const Key* pParentKey,
                      DWORD dwValueType ) :
            m_pParentKey( pParentKey ),
            m_dwValueType( dwValueType )
        {
            m_wsName = utf8ToWide( sName );
        }


        std::string Value::name() const
        {
            return wideToUtf8( m_wsName );
        }


        DWORD Value::type() const
        {
            return m_dwValueType;
        }


        std::string Value::readString() const
        {
            return m_pParentKey->readString( wideToUtf8( m_wsName ) );
        }


    } // registry
} // bp

