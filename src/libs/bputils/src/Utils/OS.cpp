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

#include "OS.h"
#include "bpstrutil.h"

#ifdef WIN32
  #include <windows.h>
  #define snprintf _snprintf_s
  typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
  LPFN_ISWOW64PROCESS fnIsWow64Process;
#else
  #ifdef MACOSX
    #include <Carbon/Carbon.h>
  #endif
#include <unistd.h>
#include <pwd.h>
#endif

bp::os::PlatformType bp::os::Platform()
{
#ifdef WIN32
    return Windows;
#elif defined(MACOSX)
    return OSX;
#elif defined(Linux)
    return Linux;
#else
    return Unknown;
#endif
}

std::string bp::os::PlatformAsString() 
{
    std::string p;
    switch (bp::os::Platform()) {
        case Windows: p = "win32"; break;
        case OSX: p = "osx"; break;
        case Linux: p = "linux"; break;
        case Unknown: p = "unknown"; break;
    }
    return p;
}

std::string bp::os::PlatformVersion()
{
    std::string version;

#ifdef WIN32
    char buf [1024];
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi,sizeof(OSVERSIONINFOEX));
	ZeroMemory(&buf,sizeof(buf));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if(!GetVersionEx((OSVERSIONINFO*)&osvi))
	{
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if(!GetVersionEx((OSVERSIONINFO*)&osvi)) {
			return std::string("unknown");
        }
    }
    snprintf(buf, sizeof(buf), "%lu.%lu.%lu",
	     osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
    version.append(buf);
#elif defined(MACOSX)
	bool success = true;

	CFURLRef urlRef = CFURLCreateWithFileSystemPath(
		NULL,
		CFSTR("/System/Library/CoreServices/SystemVersion.plist"),
		kCFURLPOSIXPathStyle,
		false);
		
	CFDataRef resourceData = NULL;
	
	SInt32 errorCode = 0;

	if(CFURLCreateDataAndPropertiesFromResource(
		NULL,
		urlRef,
		&resourceData,
		NULL,
		NULL,
		&errorCode))
	{
		CFStringRef errorString = NULL;
		CFDictionaryRef propertyList = NULL;

		propertyList = (CFDictionaryRef)CFPropertyListCreateFromXMLData(NULL, resourceData, kCFPropertyListImmutable, &errorString);
		
		if(propertyList != NULL && CFGetTypeID(propertyList) == CFDictionaryGetTypeID())
		{
			CFStringRef ver = (CFStringRef)CFDictionaryGetValue(propertyList, CFSTR("ProductVersion"));
		
			int len = CFStringGetLength(ver);
			char* text = new char[len+1];
		
			CFStringGetCString(ver, text, len+1, kCFStringEncodingUTF8);
		
			version.append(text);
			delete [] text;

			// Don't release 'version' -- releasing the dictionary will
            // release it
			CFRelease(propertyList);
		}
		else
		{
			success = false;
		}
		
		if(errorString != NULL) {
			CFRelease(errorString);
        }
	}
	else
	{
        success = false;
	}

	if(resourceData != NULL) CFRelease(resourceData);
	if(urlRef != NULL) CFRelease(urlRef);
	if(!success) version.append("unknown");
#elif defined(LINUX)
    // TODO: implement
    version.append("detection not yet implemented");
#endif
    return version;
}
        
std::string bp::os::ServicePack()
{
    // TODO: this function should be made to do something,
    //       or should be removed.
    return std::string("unknown");
}

std::string bp::os::CurrentUser()
{
    std::string usr;
#ifdef WIN32
    static const unsigned int bufSize = 32767;
	wchar_t szUser[bufSize];
	DWORD chBuff = bufSize;
	GetUserNameW(szUser,&chBuff);
    usr.append(bp::strutil::wideToUtf8(szUser));
#else
	uid_t uid;
	struct passwd *pw;
	uid = getuid();
	pw = getpwuid(uid);
    usr.append(pw->pw_name);
#endif
    return usr;
}


bool bp::os::Is64Bit()
{
	bool bIs64Bit = false;
#ifdef WIN32
	if (NULL != fnIsWow64Process)
    {
		BOOL is64bit = false;
        fnIsWow64Process(GetCurrentProcess(),&is64bit);
		bIs64Bit = (is64bit == TRUE);
	}
#elif defined (MACOSX)
	OSErr err;
	long cpuAttributes = 0;
	err = Gestalt(gestaltPowerPCProcessorFeatures, &cpuAttributes);
	if(err == noErr)
	{
		bIs64Bit = (6 << gestaltPowerPCHas64BitSupport) & cpuAttributes;
	}
#elif defined (Linux)
    // TODO: how might we reliably detect this on linux?
    bIs64Bit = false;
#endif
    return (bool) bIs64Bit;
}


bool bp::os::IsDeprecated()
{
    bool rval = false;
#ifdef MACOSX
    std::string v = PlatformVersion();
    if (v.find("10.4.") == 0) 
    {
        rval = true;
    }
#endif
    return rval;
}