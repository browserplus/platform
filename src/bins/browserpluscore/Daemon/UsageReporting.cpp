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
 * UsageReporting.cpp
 *
 * Responsible for reporting BP client usage data to the BP backend.
 *
 * Created by David Grigsby on 01/07/2008.
 * Copyright (c) 2008 Yahoo!, Inc. All rights reserved.
 */

#include "UsageReporting.h"
#include "BPUtils/bpconfig.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/InstallID.h"
#include "BPUtils/OS.h"
#include "BPUtils/ProductPaths.h"
#include "DistributionClient/DistributionClient.h"
#include "Permissions/Permissions.h"

using namespace std;
using namespace std::tr1;


// disallow usage reporting more frequently than this number of
// seconds.  For testing or broken pages, we don't want to flood
// the end user's outbound network connection
static const unsigned int USAGE_MIN_ELAPSED = 2;

//////////////////////////////
// Internal Implementation

class UsageReporter : virtual public IDistQueryListener
{
public:
    static UsageReporter* instance();
    ~UsageReporter();

    void reportPageUsage( const std::string& sUri,
                          const std::string& sUserAgent );
       
private:
    UsageReporter();

    // Configure ourself from the config file.
    bool configure();
    
    bool getDistServerUrls( const bp::config::ConfigReader& cfg,
                            bp::StrList& sUrls );

    // implementations of methods from IDistQueryListener interface
    void onTransactionFailed(unsigned int tid);
    void onPageUsageReported(unsigned int tid);

    // hold reference to self to keep boost smart pointer
    // stuff from trying to delete us
    shared_ptr<UsageReporter> m_self;

    bool m_bEnabled;
    bool m_bReportUrl;
    bool m_bReportId;
    bp::StrList m_lsDistUrls;
    
    DistQuery * m_distQuery;

    // OS version string.
    std::string m_sOS;

    // BP version string.
    std::string m_sBP;

    // a throttle which prevents us from reporting usage more
    // frequently than once every USAGE_MIN_ELAPSED seconds.
    bp::time::Stopwatch m_throttle;
};

UsageReporter*
UsageReporter::instance()
{
    static UsageReporter* s_pSingleton = NULL;

    if (!s_pSingleton)
    {
        s_pSingleton = new UsageReporter();
    }

    return s_pSingleton;
}


UsageReporter::UsageReporter() :
m_bEnabled( false ),
m_bReportUrl( false ),
m_bReportId( false )    
{
    if (!configure())
    {
        BPLOG_ERROR("Disabling UsageReporter - configuration failed");
        m_bEnabled = false;
        return;
    }
    
    m_distQuery = new DistQuery( m_lsDistUrls, PermissionsManager::get() );
    m_distQuery->setListener(this);

    // Cache os version.
    m_sOS = bp::os::PlatformAsString() + " " + bp::os::PlatformVersion();

    // Cache BP version.
    m_sBP = bp::paths::versionString();
    
    // Hold a reference to self so we don't get prematurely destroyed.
    m_self.reset(this);
}


UsageReporter::~UsageReporter()
{
    delete m_distQuery;
}


bool UsageReporter::configure()
{
    using namespace bp;
    
    bp::config::ConfigReader cfg;
    bp::file::Path cfgPath = bp::paths::getConfigFilePath();
    if (!cfg.load( cfgPath ))
    {
        BPLOG_ERROR_STRM("Couldn't read config file at: " << cfgPath);
        return false;
    }

    std::string sUrl;
    if (!cfg.getStringValue("DistServer", sUrl))
    {
        BPLOG_ERROR_STRM("No DistServerUrl key in config file at: " << cfgPath);
        return false;
    }
    m_lsDistUrls.push_back( sUrl );
    
    // Read our config map if it's present.
    // If map (or subvalues) is/are absent we'll use our ctor defaults.
    const Map* pMap;
    if (cfg.getJsonMap( "UsageReporting", pMap ))
    {
        bool bVal;
        if (pMap->getBool( "enabled", bVal ))
        {
            m_bEnabled = bVal;
        }

        if (pMap->getBool( "url", bVal ))
        {
            m_bReportUrl = bVal;
        }

        if (pMap->getBool( "id", bVal ))
        {
            m_bReportId = bVal;
        }
    }

    return true;
}


void UsageReporter::reportPageUsage( const std::string& sUrl,
                                     const std::string& sUserAgent )
{
    if (!m_bEnabled)
    {
        BPLOG_INFO("Skipping reportPageUsage - UsageReporter disabled");
        return;
    }

    // throttle usage
    if (m_throttle.running() && m_throttle.elapsedSec() < USAGE_MIN_ELAPSED)
    {
        BPLOG_INFO_STRM("Skipping usage reporting - last report was only "
                        << m_throttle.elapsedSec() << "s ago");
        return;
    }
    m_throttle.reset();
    m_throttle.start();

    unsigned int tid = m_distQuery->reportPageUsage(
        m_sOS, m_sBP, (m_bReportUrl ? sUrl : ""),
        (m_bReportId ? bp::plat::getInstallID() : ""), sUserAgent);

    if (tid == 0)
    {
        BPLOG_ERROR("DistQuery::reportUsage returned tid==0");
        return;
    }
}

void
UsageReporter::onTransactionFailed(unsigned int)
{
    BPLOG_INFO("Usage reporting failed.");
}

void
UsageReporter::onPageUsageReported(unsigned int)
{
    BPLOG_INFO("Usage reporting succeeded.");
}


////////////////////
// Public Methods
//

namespace bp {
namespace usage {

void reportPageUsage( const std::string& sUrl,
                      const std::string& sUserAgent )
{
    UsageReporter::instance()->reportPageUsage( sUrl, sUserAgent );
}

} // usage
} // bp

