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
 *  bplocalizaton.cpp
 *      Localization utils
 *  
 *  Created by Gordon Durand on Thu March 27 2008.
 *  Copyright 2008 Yahoo! Inc. All rights reserved.
 */

#include "bplocalization.h"
#include <stdlib.h>
#include "bpconfig.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bpsemanticversion.h"
#include "ProductPaths.h"


using namespace std;

#ifdef WIN32
#include <windows.h>
#elif defined(MACOSX)
#include <CoreFoundation/CoreFoundation.h>
#endif


std::string
bp::localization::getUsersLocale()
{
    std::string locale;
#ifdef WINDOWS
    LANGID ulid = GetUserDefaultLangID();

	// now convert this crap into a locale string
    switch (ulid & 0x3F) {
        case LANG_AFRIKAANS: locale.append("af-ZA"); break;
        case LANG_ALBANIAN: locale.append("sq-AL"); break;
        case LANG_ARABIC:
            locale.append("ar");
            switch (ulid >> 10) {
                case SUBLANG_ARABIC_BAHRAIN: locale.append("-BH"); break;
                case SUBLANG_ARABIC_EGYPT: locale.append("-EG"); break;
                case SUBLANG_ARABIC_IRAQ: locale.append("-IQ"); break;
                case SUBLANG_ARABIC_JORDAN: locale.append("-JO"); break;
                case SUBLANG_ARABIC_KUWAIT: locale.append("-KW"); break;
                case SUBLANG_ARABIC_LEBANON: locale.append("-LB"); break;
                case SUBLANG_ARABIC_LIBYA: locale.append("-LY"); break;
                case SUBLANG_ARABIC_MOROCCO: locale.append("-MA"); break;
                case SUBLANG_ARABIC_OMAN: locale.append("-OM"); break;
                case SUBLANG_ARABIC_QATAR: locale.append("-QA"); break;
                case SUBLANG_ARABIC_SAUDI_ARABIA: locale.append("-SA"); break;
                case SUBLANG_ARABIC_SYRIA: locale.append("-SY"); break;
                case SUBLANG_ARABIC_TUNISIA: locale.append("-TN"); break;
                case SUBLANG_ARABIC_UAE: locale.append("-AE"); break;
                case SUBLANG_ARABIC_YEMEN: locale.append("-YE"); break;
            }
            break;
        case LANG_ARMENIAN: locale.append("hy-AM"); break;
        case LANG_ASSAMESE: locale.append("as-IN"); break;
        case LANG_AZERI: locale.append("az-AZ"); break;
        case LANG_BASQUE: locale.append("eu-ES"); break;
        case LANG_BELARUSIAN: locale.append("be-BY"); break;
        case LANG_BENGALI: locale.append("bn-IN"); break;
        case LANG_BULGARIAN: locale.append("bg-BG"); break;
        case LANG_CATALAN: locale.append("ca-ES"); break;
        case LANG_CHINESE:
            locale.append("zh");
            switch(ulid >> 10) {
                case SUBLANG_CHINESE_HONGKONG: locale.append("-Hant-HK"); break;
                case SUBLANG_CHINESE_MACAU: locale.append("-MO"); break;
                case SUBLANG_CHINESE_SINGAPORE: locale.append("-SG"); break;
                case SUBLANG_CHINESE_SIMPLIFIED: locale.append("-Hans-CN"); break;
                case SUBLANG_CHINESE_TRADITIONAL: locale.append("-Hant-TW"); break;
            }
            break;            
//         case LANG_CROATIAN:
//             locale.append("hr");
//             switch(ulid >> 10) {
//                 case SUBLANG_CROATIAN_BOSNIA_HERZEGOVINA_LATIN: locale.append("-BA"); break;
//                 case SUBLANG_CROATIAN_CROATIA: locale.append("-HR"); break;
//             }
//             break;
        case LANG_CZECH: locale.append("cs-CZ"); break;
        case LANG_DANISH: locale.append("da-DK"); break;
        case LANG_DUTCH:
            locale.append("nl");
            switch(ulid >> 10) {
                case SUBLANG_DUTCH_BELGIAN: locale.append("-BE"); break;
                case SUBLANG_DUTCH: locale.append("-NL"); break;
            }
            break;
        case LANG_ENGLISH:
            locale.append("en");
            switch(ulid >> 10) {
                case SUBLANG_ENGLISH_AUS: locale.append("-AU"); break;
                case SUBLANG_ENGLISH_BELIZE: locale.append("-BE"); break;
                case SUBLANG_ENGLISH_CAN: locale.append("-CA"); break;
                case SUBLANG_ENGLISH_CARIBBEAN: locale.append("-029"); break;
                case SUBLANG_ENGLISH_INDIA: locale.append("-IN"); break;
                case SUBLANG_ENGLISH_EIRE: locale.append("-IE"); break;
           //     case SUBLANG_ENGLISH_IRELAND: locale.append("-IE"); break;
                case SUBLANG_ENGLISH_JAMAICA: locale.append("-JM"); break;
                case SUBLANG_ENGLISH_MALAYSIA: locale.append("-MY"); break;
                case SUBLANG_ENGLISH_NZ: locale.append("-NZ"); break;
                case SUBLANG_ENGLISH_PHILIPPINES: locale.append("-PH"); break;
                case SUBLANG_ENGLISH_SINGAPORE: locale.append("-SG"); break;
                case SUBLANG_ENGLISH_SOUTH_AFRICA: locale.append("-ZA"); break;
                case SUBLANG_ENGLISH_TRINIDAD: locale.append("-TT"); break;
                case SUBLANG_ENGLISH_UK: locale.append("-GB"); break;
                case SUBLANG_ENGLISH_US: locale.append("-US"); break;
                case SUBLANG_ENGLISH_ZIMBABWE: locale.append("-ZW"); break;
            }
            break;
        case LANG_ESTONIAN: locale.append("et-EE"); break;
        case LANG_FAEROESE: locale.append("fo-FO"); break;
        case LANG_FARSI: locale.append("fa-IR"); break;
        case LANG_FINNISH: locale.append("fi-FI"); break;
        case LANG_FRENCH:
            locale.append("fr");
            switch(ulid >> 10) {
                case SUBLANG_FRENCH_BELGIAN: locale.append("-BE"); break;
                case SUBLANG_FRENCH_CANADIAN: locale.append("-CA"); break;
                case SUBLANG_FRENCH: locale.append("-FR"); break;
                case SUBLANG_FRENCH_LUXEMBOURG: locale.append("-LU"); break;
                case SUBLANG_FRENCH_MONACO: locale.append("-MC"); break;
                case SUBLANG_FRENCH_SWISS: locale.append("-CH"); break;
            }
            break;
        case LANG_GEORGIAN: locale.append("ka-GE"); break;
        case LANG_GERMAN:
            locale.append("de");
            switch(ulid >> 10) {
                case SUBLANG_GERMAN_AUSTRIAN: locale.append("-AT"); break;
                case SUBLANG_GERMAN: locale.append("-DE"); break;
                case SUBLANG_GERMAN_LIECHTENSTEIN: locale.append("-LI"); break;
                case SUBLANG_GERMAN_LUXEMBOURG: locale.append("-LU"); break;
                case SUBLANG_GERMAN_SWISS: locale.append("-CH"); break;
            }
            break;
        case LANG_GREEK: locale.append("el-GR"); break;
        case LANG_GUJARATI: locale.append("gu-IN"); break;
        case LANG_HEBREW: locale.append("he-IL"); break;
        case LANG_HINDI: locale.append("hi-IN"); break;
        case LANG_HUNGARIAN: locale.append("hu-HU"); break;
        case LANG_ICELANDIC: locale.append("is-IS"); break;
        case LANG_INDONESIAN: locale.append("id-ID"); break;
        case LANG_ITALIAN:
            locale.append("it");
            switch(ulid >> 10) {
                case SUBLANG_ITALIAN: locale.append("-IT"); break;
                case SUBLANG_ITALIAN_SWISS: locale.append("-CH"); break;
            }
            break;
        case LANG_JAPANESE: locale.append("ja-JP"); break;
        case LANG_KANNADA: locale.append("kn-IN"); break;
        case LANG_KAZAK: locale.append("kk-KZ"); break;
        case LANG_KONKANI: locale.append("kok-IN"); break;
        case LANG_KOREAN: locale.append("ko-KR"); break;
        case LANG_LATVIAN: locale.append("lv-LV"); break;
        case LANG_LITHUANIAN: locale.append("lt-LT"); break;
        case LANG_MACEDONIAN: locale.append("mk-MK"); break;
        case LANG_MALAY: locale.append("ms-BN"); break;
        case LANG_MALAYALAM: locale.append("ml-IN"); break;
        case LANG_MARATHI: locale.append("mr-IN"); break;
        case LANG_NEPALI: locale.append("ne-NP"); break;
        case LANG_NORWEGIAN: locale.append("no-NO"); break;
        case LANG_ORIYA: locale.append("or-IN"); break;
        case LANG_POLISH: locale.append("pl-PL"); break;
        case LANG_PORTUGUESE:
            locale.append("pt");
            switch(ulid >> 10) {
                case SUBLANG_PORTUGUESE_BRAZILIAN: locale.append("-BR"); break;
                case SUBLANG_PORTUGUESE: locale.append("-PT"); break;
            }
            break;
        case LANG_PUNJABI: locale.append("pa-IN"); break;
        case LANG_ROMANIAN: locale.append("ro-RO"); break;
        case LANG_RUSSIAN: locale.append("ru-RU"); break;
        case LANG_SANSKRIT: locale.append("sa-IN"); break;
        case LANG_SERBIAN:
            locale.append("sr");
            switch(ulid >> 10) {
                case SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_CYRILLIC: locale.append("-BA"); break;
                case SUBLANG_SERBIAN_BOSNIA_HERZEGOVINA_LATIN: locale.append("-BA"); break;
                case SUBLANG_SERBIAN_CROATIA: locale.append("-HR"); break;
                case SUBLANG_SERBIAN_CYRILLIC: locale.append("-CS"); break;
                case SUBLANG_SERBIAN_LATIN: locale.append("-CS"); break;
            }
            break;
        case LANG_SLOVAK: locale.append("sk-SK"); break;
        case LANG_SLOVENIAN: locale.append("sl-SI"); break;
        case LANG_SPANISH:
            locale.append("es");
            switch(ulid >> 10) {
                case SUBLANG_SPANISH_ARGENTINA: locale.append("-AR"); break;
                case SUBLANG_SPANISH_BOLIVIA: locale.append("-BO"); break;
                case SUBLANG_SPANISH_CHILE: locale.append("-CL"); break;
                case SUBLANG_SPANISH_COLOMBIA: locale.append("-CO"); break;
                case SUBLANG_SPANISH_COSTA_RICA: locale.append("-CR"); break;
                case SUBLANG_SPANISH_DOMINICAN_REPUBLIC: locale.append("-DO"); break;
                case SUBLANG_SPANISH_ECUADOR: locale.append("-EC"); break;
                case SUBLANG_SPANISH_EL_SALVADOR: locale.append("-SV"); break;
                case SUBLANG_SPANISH_GUATEMALA: locale.append("-GT"); break;
                case SUBLANG_SPANISH_HONDURAS: locale.append("-HN"); break;
                case SUBLANG_SPANISH_MEXICAN: locale.append("-MX"); break;
                case SUBLANG_SPANISH_NICARAGUA: locale.append("-NI"); break;
                case SUBLANG_SPANISH_PANAMA: locale.append("-PA"); break;
                case SUBLANG_SPANISH_PARAGUAY: locale.append("-PY"); break;
                case SUBLANG_SPANISH_PERU: locale.append("-PE"); break;
                case SUBLANG_SPANISH_PUERTO_RICO: locale.append("-PR"); break;
                case SUBLANG_SPANISH: locale.append("-ES"); break;
                case SUBLANG_SPANISH_US: locale.append("-US"); break;
                case SUBLANG_SPANISH_URUGUAY: locale.append("-UY"); break;
                case SUBLANG_SPANISH_VENEZUELA: locale.append("-VE"); break;
            }
            break;
        case LANG_SWAHILI: locale.append("sw-KE"); break;
        case LANG_SWEDISH:
            locale.append("sv");
            switch(ulid >> 10) {
                case SUBLANG_SWEDISH_FINLAND: locale.append("-FI"); break;
                case SUBLANG_SWEDISH: locale.append("-SE"); break;
                //case SUBLANG_SWEDISH_SWEDEN                : locale.append("-SE"); break;
            }
            break;
        case LANG_TAMIL: locale.append("ta-IN"); break;
        case LANG_TATAR: locale.append("tt-RU"); break;
        case LANG_TELUGU: locale.append("te-IN"); break;
        case LANG_THAI: locale.append("th-TH"); break;
        case LANG_TURKISH: locale.append("tr-TR"); break;
        case LANG_UKRAINIAN: locale.append("uk-UA"); break;
        case LANG_URDU: locale.append("ur-PK"); break;
        case LANG_UZBEK: locale.append("uz-UZ"); break;
        case LANG_VIETNAMESE: locale.append("vi-VN"); break;
    }

#elif defined (MACOSX)
    locale.clear();

    // first let's get the raw language in a canonical format 
    CFStringRef lang = NULL;
    CFStringRef country = NULL;
    
    CFArrayRef appleLangArr =
        (CFArrayRef) CFPreferencesCopyValue(CFSTR("AppleLanguages"),
                                            kCFPreferencesAnyApplication,
                                            kCFPreferencesCurrentUser,
                                            kCFPreferencesAnyHost);
    if (appleLangArr)  {
        // Take the topmost language.
        CFStringRef s1 = (CFStringRef) CFArrayGetValueAtIndex(appleLangArr, 0);
        lang = CFLocaleCreateCanonicalLanguageIdentifierFromString(
            kCFAllocatorDefault, s1);
    }

    // now let's get the country
    CFLocaleRef loc = CFLocaleCopyCurrent();
    if (loc != NULL) {
        country = (CFStringRef) CFLocaleGetValue(loc, kCFLocaleCountryCode);
    }

    if (lang != NULL && country != NULL) {
        char buf[128];
        if (CFStringGetCString(lang, buf, sizeof(buf),
                               kCFStringEncodingUTF8))
        {
            locale.append(buf);
        }
        locale.append("-");
        if (CFStringGetCString(country, buf, sizeof(buf),
                               kCFStringEncodingUTF8))
        {
            locale.append(buf);
        }
    }

    // fallback to environment
    if (locale.empty()) {
        char * l = getenv("LANG");
        if (l != NULL) locale = l;
    }
#else
    // use ENV ?
#warning "not implemented on platform!  this code will abort() at runtime!"
    abort();
#endif

    if (locale.empty()) locale.append("en-US");
    
    return locale;
}


bool
bp::localization::getLocalizedString(const string& key,
                                     const string& locale,
                                     string& outVal,
                                     unsigned int major,
                                     unsigned int minor,
                                     unsigned int micro,
                                     bool useUpdateCache)
{
    boost::filesystem::path path = bp::paths::getLocalizedStringsPath(major, minor, micro, 
                                                                      useUpdateCache);
    return getLocalizedString(key, locale, outVal, path);
}


bool
bp::localization::getLocalizedString(const string& key,
                                     const string& locale,
                                     string& outVal,
                                     const boost::filesystem::path& stringsPath)
{
    bool found = false;
    bp::config::ConfigReader reader;
    if (reader.load(stringsPath)) {
        vector<string> locales = getLocaleCandidates(locale);
        for (unsigned int i = 0; i < locales.size() && !found; ++i) {
            std::string newKey = key + "/" + locales[i];
            found = reader.getStringValue(newKey, outVal);
        }
    }
    if (!found) {
        outVal = key;
    }
    return(found);
}

std::map<std::string, std::string>
bp::localization::getLocalizations(const std::string& key)
{
    std::map<std::string, std::string> rMap;

    boost::filesystem::path path = bp::paths::getLocalizedStringsPath();
    bp::config::ConfigReader reader;
    const bp::Map * m;
    if (reader.load(path) && reader.getJsonMap(key, m))
    {
        bp::Map::Iterator i(*m);
        const char * l;
        while (NULL != (l = i.nextKey())) {
            std::string v;
            if (m->getString(l, v)) {
                rMap[l] = v;
            }
        }
    }

    return rMap;
}

vector<string> 
bp::localization::getLocaleCandidates(const string& locale)
{    
    // locale format is language[-country][.codeset][@modifier]
    // successively strip elements
    string tmp = locale;
    
    // sigh, must deal with _ as a variant of -
    // some people just don't follow the rules
    for (size_t i = 0; i < tmp.length(); i++) {
        if (tmp[i] == '_') {
            tmp.replace(i, 1, "-");
        }
    }
    
    vector<string> rval;
    rval.push_back(tmp);
    string::size_type pos = tmp.find('@');
    if (pos != string::npos) {
        tmp = tmp.substr(0, pos);
        rval.push_back(tmp);
    }
    pos = tmp.find('.');
    if (pos != string::npos) {
        tmp = tmp.substr(0, pos);
        rval.push_back(tmp);
    }

    bool doneWithCountry = false;
    do {
        pos = tmp.rfind('-');
        if (pos != string::npos) {
            tmp = tmp.substr(0, pos);
            rval.push_back(tmp);
        } else {
            doneWithCountry = true;
        }
    } while (!doneWithCountry);

    if (rval[rval.size()-1].compare("en")) {
        rval.push_back("en");
    }
    return rval;
}


boost::filesystem::path
bp::localization::getLocalizedUIPath(const boost::filesystem::path & uiDir,
                                     const std::string & locale)
{
    // get a list of subdirs, get a list of locale candidates, and run
    // through the locale candidates returning the first subdir that
    // exists
    std::vector<std::string> candidates = getLocaleCandidates(locale);
    boost::filesystem::path path;

    for (unsigned int i = 0; i < candidates.size(); i++)
    {
        boost::filesystem::path p = uiDir / candidates[i];
        if (bp::file::isDirectory(p)) {
            path = p;
            break;
        }
    }

    return path;
}


