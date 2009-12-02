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

#include "BPInstaller/BPInstaller.h"
#include <string>
#include <list>
#include "BPUtils/ARGVConverter.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bplocalization.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bprunloop.h"
#include "BPUtils/ProcessLock.h"
#include "BPUtils/ProductPaths.h"
#include "InstallerSkin.h"
#include "InstallerSkinVerbose.h"
#include "InstallerSkinGUI.h"
#include "InstallerRunner.h"

using namespace std;
using namespace std::tr1;
using namespace bp::file;
using namespace bp::install;

// localization keys
//
static const char* kPlatformDownloading = "kPlatformDownloading";
static const char* kServicesDownloading = "kServicesDownloading";

// some words about progress percentage bucketing:  
// 0%-5% is startup
// 5%-50% is attaining browseplus bpkg
// 50%-55% is installation prep
// 55%-90% is installation
// 90%-100% is finishing up

class AsyncHelper : virtual public IFetcherListener,
                    public enable_shared_from_this<AsyncHelper>
{
public:
    typedef enum {
        eGetPlatformVersionAndSize,
        eDownloadPlatform,
        eDownloadServices
    } tCommand;

    AsyncHelper(const Path& keyPath,
				const list<CoreletRequireStatement>& services,
                const list<string>& distroServers,
                const Path& destDir,
                bp::runloop::RunLoop* rl)
    : m_fetcher(keyPath, distroServers, destDir),
      m_services(services), m_rl(rl) 
    {
    }
    
    void setListener(weak_ptr<IFetcherListener> l) 
    {
        m_listener = l;
    }
    
    void getPlatformVersionAndSize() 
    {
        m_fetcher.setListener(weak_ptr<IFetcherListener>(shared_from_this()));
        m_fetcher.getPlatformVersionAndSize();
    }

    void downloadPlatform() 
    {
        m_fetcher.setListener(weak_ptr<IFetcherListener>(shared_from_this()));
        m_fetcher.getPlatform();
    }

    void downloadServices() 
    {
        m_fetcher.setListener(weak_ptr<IFetcherListener>(shared_from_this()));
        m_fetcher.getServices(m_services);
    }

    string platformVersion() const 
    {
        return m_fetcher.platformVersion();
    }

    size_t platformSize() const 
    {
        return m_fetcher.platformSize();
    }
    
    virtual void onTransactionFailed(unsigned int tid,
                                     const string& msg) 
    {
        shared_ptr<IFetcherListener> l = m_listener.lock();
        if (l) {
            l->onTransactionFailed(tid, msg);
        }
        m_rl->stop();
    }
    
    virtual void onDownloadProgress(unsigned int tid,
                                    const string& item,
                                    unsigned int pct) 
    {
        shared_ptr<IFetcherListener> l = m_listener.lock();
        if (l) {
            l->onDownloadProgress(tid, item, pct);
        } 
    }
    
    virtual void onServicesDownloaded(unsigned int tid) 
    {
        shared_ptr<bp::install::IFetcherListener> l = m_listener.lock();
        if (l) {
            l->onServicesDownloaded(tid);
        }
        m_rl->stop();
    }
    
    virtual void onPlatformDownloaded(unsigned int tid)
   {
        shared_ptr<bp::install::IFetcherListener> l = m_listener.lock();
        if (l) {
            l->onPlatformDownloaded(tid);
        }
        m_rl->stop();
    }
    
    virtual void onPlatformVersionAndSize(unsigned int tid,
                                          const string& version,
                                          size_t size)
    {
        shared_ptr<bp::install::IFetcherListener> l = m_listener.lock();
        if (l) {
            l->onPlatformVersionAndSize(tid, version, size);
        }
        m_rl->stop();
    }

private:
    bp::install::Fetcher m_fetcher;
    list<CoreletRequireStatement> m_services;
    bp::runloop::RunLoop* m_rl;
    weak_ptr<bp::install::IFetcherListener> m_listener;

};


static pair<string, size_t>
runIt(AsyncHelper::tCommand command,
      weak_ptr<IFetcherListener> listener,
	  const Path& keyPath,
      const list<string>& servers,
      const Path& destDir,
      list<CoreletRequireStatement> services = list<CoreletRequireStatement>())
{ 
    bp::runloop::RunLoop rl;
    rl.init();
    
    shared_ptr<AsyncHelper> async(new AsyncHelper(keyPath, services, servers,
                                                  destDir, &rl));
    async->setListener(listener);
    switch (command) {
    case AsyncHelper::eGetPlatformVersionAndSize:
        async->getPlatformVersionAndSize();
        break;
    case AsyncHelper::eDownloadPlatform:
        async->downloadPlatform();
        break;
    case AsyncHelper::eDownloadServices:
        async->downloadServices();
        break;
    default:
        BP_THROW("invalid command in runIt");
    }
    rl.run();
    pair<string, size_t> rval(async->platformVersion(),
                              async->platformSize());
    async.reset();
    return rval;
}


class InstallManager : virtual public IFetcherListener,
                       virtual public IInstallerListener,
                       virtual public IInstallerSkinListener,
                       public enable_shared_from_this<InstallManager>
{
public:
    InstallManager(const Path & exeDir,
                   const Path & destDir,
                   const Path & updatePkg,
                   const Path& keyPath,
                   const list<string>& servers,
                   const string& version,
                   const list<CoreletRequireStatement> & services,
                   const string& permissions,
                   const string& autoUpdatePermissions,
                   shared_ptr<InstallerSkin> skin,
                   bp::runloop::RunLoop * rl,
                   unsigned int width,
                   unsigned int height,
                   std::string title)
        : m_exeDir(exeDir), m_destDir(destDir), m_updatePkg(updatePkg),
          m_keyPath(keyPath), m_servers(servers), m_platformVersion(version),
          m_platformSize(0), m_services(services), m_permissions(permissions),
          m_autoUpdatePermissions(autoUpdatePermissions), m_skin(skin), m_rl(rl), 
          m_width(width), m_height(height), m_title(title),
          m_installerLock(NULL), m_state(ST_Started),
          m_downloadingServices(false)
    {
        if (m_skin != NULL) m_skin->setListener(this);
    }

    ~InstallManager()
    {
        if (m_installerLock != NULL) {
            bp::releaseProcessLock(m_installerLock);
            m_installerLock = NULL;
        }
    }

    void run() 
    {
        // verify another instance of the installer is not running
        m_installerLock =
            bp::acquireProcessLock(false, std::string("BrowserPlusInstaller"));
        if (m_installerLock == NULL) {
            m_skin->errorMessage(Installer::getString(Installer::kInstallerAlreadyRunning));
        }

        assert(m_state == ST_Started);
        m_state = ST_WaitingToBegin;
        BPLOG_INFO_STRM("calling startUp (" << m_skin.get() << ")");
        if (m_skin != NULL) m_skin->startUp(m_width, m_height, m_title);
        else beginInstall();
    }
          
private:
    Path m_exeDir;
    Path m_destDir;
    Path m_updatePkg;
    Path m_keyPath;
    list<string> m_servers;
    string m_platformVersion;
    size_t m_platformSize;
    list<CoreletRequireStatement> m_services;
    string m_permissions;
    string m_autoUpdatePermissions;
    shared_ptr<InstallerSkin> m_skin;
    bp::runloop::RunLoop * m_rl;
    shared_ptr<InstallerRunner> m_runner;
    unsigned int m_width, m_height;
    std::string m_title;
    
    // a lock to protect against multiple instances
    bp::ProcessLock m_installerLock;

    // installation state machine
    enum {
        // Just started up
        ST_Started, 
        // Waiting for the skin to let us begin the install (user confirm)
        ST_WaitingToBegin, 
        // Actively installing bits
        ST_Installing, 
        // Waiting for the skin to let us shut down the install
        ST_WaitingToEnd, 
        // all done, yum.
        ST_AllDone,
        // we got canceled :(
        ST_Canceled 
    } m_state;
    bool m_downloadingServices;

    // do the body of the installation.  This should be broken up into
    // asynchronous steps
    void doInstall()
    {
        // used to format messages for the output skin
        stringstream ss; 
       
        if (m_skin) m_skin->progress(1);

        // Fetch and unpack platform into platformDir
        // where Installer will find it;
        if (m_skin) {
            m_skin->statusMessage(Installer::getString(kPlatformDownloading));
        }
        Path platformDir = m_destDir;
        if (m_updatePkg.empty()) {
            {
                ss.clear();
                ss << "fetch to " << m_destDir;
                if (m_skin) m_skin->debugMessage(ss.str());
                ss.clear();
            }
            
            // Get latest platform info
            pair<string, size_t> p = runIt(AsyncHelper::eGetPlatformVersionAndSize,
                                           shared_from_this(), m_keyPath, m_servers,
                                           m_destDir);
            m_platformVersion = p.first;
            m_platformSize = p.second;
            platformDir /= m_platformVersion;

            if (m_skin) m_skin->progress(2);

            // download latest platform
            (void) runIt(AsyncHelper::eDownloadPlatform,
                         shared_from_this(), m_keyPath, m_servers,
                         m_destDir);


        } else {
            if (m_skin) m_skin->progress(5);

            platformDir /= m_platformVersion;
            Path destPkg = m_destDir / m_updatePkg.filename();
            {
                ss.clear();
                ss << "installing from " << destPkg;
                if (m_skin) m_skin->debugMessage(ss.str());
                ss.clear();
            }

            try {
                boost::filesystem::create_directories(m_destDir);
            } catch(const tFileSystemError&) {
                BP_THROW("unable to create " + m_destDir.externalUtf8());
            }
            (void) remove(destPkg);
            if (!copy(m_updatePkg, destPkg)) {
                BP_THROW("unable to copy " + m_updatePkg.externalUtf8()
                         + " -> " + destPkg.externalUtf8());
            }
            if (m_skin) m_skin->progress(15);
            PlatformUnpacker unpacker(destPkg, m_destDir,
                                      m_platformVersion, m_keyPath);
            string errMsg;
            if (!unpacker.unpack(errMsg)) {
                BP_THROW("unpack failed: " + errMsg);
            }
            if (m_skin) m_skin->progress(25);
            if (!unpacker.install(errMsg)) {
                BP_THROW(Installer::getString(Installer::kErrorEncountered)
                         + errMsg);
            }
            remove(destPkg);
            if (m_skin) m_skin->progress(35);
        }

        if (m_skin) m_skin->progress(41);    

        // Now install requested services into platform update dir
        // where Installer will find them
        //
        if (!m_services.empty()) {
            m_downloadingServices = true;
            if (m_skin) {
                m_skin->statusMessage(Installer::getString(kServicesDownloading));
            }
            (void) runIt(AsyncHelper::eDownloadServices,
                         shared_from_this(), m_keyPath, m_servers,
                         platformDir, m_services);
        }

        if (m_skin) m_skin->progress(66);    

        // Now install pre-seeded permissions into platform dir
        // where Installer will find them
        // 
        if (!m_permissions.empty()) {
            Path permsPath = platformDir / "permissions" / "configDomainPermissions";
            if (!bp::strutil::storeToFile(permsPath, m_permissions)) {
                BP_THROW("unable to write " + permsPath.externalUtf8());
            }
        }

        // Now install pre-seeded auto-update permissions into platform dir
        // where Installer will find them
        // 
        if (!m_autoUpdatePermissions.empty()) {
            Path permsPath = platformDir / "permissions" / "configAutoUpdatePermissions";
            if (!bp::strutil::storeToFile(permsPath, m_autoUpdatePermissions)) {
                BP_THROW("unable to write " + permsPath.externalUtf8());
            }
        }

        if (m_skin) m_skin->progress(67);    

        // Copy uninstallers into  platformDir
        Path uninsScript = bp::paths::getUninstallerPath().filename();
        Path uninsSrc = m_exeDir / uninsScript;
        if (boost::filesystem::exists(uninsSrc)) {
            Path uninsDst = platformDir / uninsScript;
            if (!copy(uninsSrc, uninsDst)) {
                BP_THROW("unable to copy " + uninsSrc.externalUtf8()
                         + " -> " + uninsDst.externalUtf8());
            }
        }

        if (m_skin) m_skin->progress(69);

        // platformDir is all set up, install from it.
        //
        m_runner.reset(new InstallerRunner);
        m_runner->setListener(this);
        m_runner->start(platformDir, false);
    }
    
    void shutdown()
    {
        if (m_state == ST_WaitingToEnd) {
            m_state = ST_AllDone;
            m_rl->stop();            
            bp::file::remove(m_destDir);
            if (m_skin) m_skin->ended();
            doExit(0);
        }
    }
    
    void doExit(int status)
    {
        BPLOG_DEBUG_STRM("exit with status " << status);
#ifdef MACOSX
        if (m_exeDir.utf8().find("/Volumes/BrowserPlusInstaller") == 0) {
            BPLOG_DEBUG("detach /Volumes/BrowserPlusInstaller");
            system("hdiutil detach /Volumes/BrowserPlusInstaller -force &");
        } else {
            BPLOG_DEBUG_STRM(m_exeDir << " not mounted at /Volumes/BrowserPlusInstaller");
        }
#endif
        exit(status);
    }

    void beginInstall() 
    {
        if (m_state == ST_WaitingToBegin) {
            m_state = ST_Installing;
            doInstall();        
        }
    }

    void cancelInstallation()
    {
        m_state = ST_Canceled;
        m_rl->stop();        
        bp::file::remove(m_destDir);
        if (m_skin) m_skin->ended();
        doExit(0);
    }

    virtual void onTransactionFailed(unsigned int tid,
                                     const string& msg)
    {
        if (m_skin) {
            m_skin->errorMessage(Installer::getString(Installer::kErrorEncountered));
        }
    }
    
    // IFetcherListener overrides

    virtual void onDownloadProgress(unsigned int tid,
                                    const string& file,
                                    unsigned int pct)
    {
        if (m_skin) {
            if (m_downloadingServices) {
                // scale service download progress between 42 - 65
                pct = (unsigned int) ((((double) pct / 100.0) * 23.0) + 42);
                m_skin->progress(pct);
            }
            else 
            {
                // scale platform download progress between 3 - 40
                pct = (unsigned int) ((((double) pct / 100.0) * 37.0) + 3);
                m_skin->progress(pct);
            }
        }
    }
    
    virtual void onServicesDownloaded(unsigned int tid)
    {
    }
    
    virtual void onPlatformDownloaded(unsigned int tid)
    {
    }

    virtual void onPlatformVersionAndSize(unsigned int tid,
                                          const string& version,
                                          size_t size)
    {
        // empty
    }

    // IInstallerListener overrides
    
    virtual void onStatus(const string& msg)
	{
        if (m_skin) {
            m_skin->statusMessage(msg);
        }
	}

    virtual void onError(const string& msg)
	{
        if (m_skin) {
            m_skin->errorMessage(msg);
        }
	}

    virtual void onProgress(unsigned int pct)
	{
        if (m_skin) {
            // scale installation progress between 70 - 100
            pct = (unsigned int) ((((double) pct / 100.0) * 30.0) + 70);
            m_skin->progress(pct);
        }

        if (pct == 100) {
            m_state = ST_WaitingToEnd;
            if (m_skin) m_skin->allDone();
            else shutdown();
        }
	}

};


static string
versionFromPackage(const Path& pkg)
{
    // name must be BrowserPlus_x.x.xx.bpkg
    Path fname = pkg.filename();
    string pkgStr = fname.utf8();
    string rval;
    size_t start = pkgStr.find("_");
    size_t end = pkgStr.find(".bpkg");
    if (start != string::npos && end != string::npos && start < end) {
        rval = pkgStr.substr(start + 1, end - start - 1);
    }
    return rval;
}

        
static void
readConfig(const Path& configPath,
		   list<string>& servers,
           Path& updatePackage,
           string& version,
           list<CoreletRequireStatement>& services,
           string& permissions,
           string& autoUpdatePermissions,
           unsigned int& width,
           unsigned int& height,
           string& title)
{
    string json, errMsg;
    if (!bp::strutil::loadFromFile(configPath, json)) {
        BP_THROW("unable to read " + configPath.externalUtf8());
    }
    bp::Object* configObj = bp::Object::fromPlainJsonString(json, &errMsg);
    if (!configObj) {
        BP_THROW(errMsg);
    }
    bp::Map* configMap = dynamic_cast<bp::Map*>(configObj);
    if (!configMap) {
        BP_THROW("bad config file format");
    }
    
    // get distro servers
    const bp::List* l = NULL;
    if (!configMap->getList("distroServers", l)) {
        BP_THROW("bad config file format");
    }
    
    for (unsigned int i = 0; i < l->size(); i++) {
        const bp::String* s = dynamic_cast<const bp::String*>(l->value(i));
        if (!s) {
            BP_THROW("bad config file format");
        }
        servers.push_back(s->value());
    }
    
    // get included bpkg if present
    string updatePackageStr;
    if (configMap->getString("package", updatePackageStr)) {
        updatePackage = updatePackageStr;
        version = versionFromPackage(updatePackage);
        if (version.empty()) {
            BPLOG_ERROR("Package in config file not of form BrowserPlus_x.x.xx.bpkg");
            updatePackage.clear();
        }
    }
    
    // Get list of requested services
    if (configMap->getList("services", l)) {
        for (unsigned int i = 0; i < l->size(); i++) {
            CoreletRequireStatement req;
            const bp::Map* m = dynamic_cast<const bp::Map*>(l->value(i));
            if (!m) {
                BP_THROW("bad config file format");
            }
            const bp::String* s = dynamic_cast<const bp::String*>(m->get("service"));
            if (!s) {
                BP_THROW("bad config file format");
            }
            req.m_name = s->value();
            s = dynamic_cast<const bp::String*>(m->get("version"));
            if (s) {
                req.m_version = s->value();
            }
            s = dynamic_cast<const bp::String*>(m->get("minversion"));
            if (s) {
                req.m_minversion = s->value();
            }
            services.push_back(req);
        }
    }
    
    // Get permissions to pre-seed
    //
    const bp::Map* m = NULL;
    if (configMap->getMap("permissions", m)) {
        permissions = m->toPlainJsonString(true);
    }

    // Get autoupdate permissions to pre-seed
    //
    m = NULL;
    if (configMap->getMap("autoUpdatePermissions", m)) {
        autoUpdatePermissions = m->toPlainJsonString(true);
    }

    // Installer UI window properties
    //
    if (configMap->getMap("window", m)) {
        if (m->has("width", BPTInteger)) {
            width = (unsigned int) ((long long) *(m->get("width")));
        }
        if (m->has("height", BPTInteger)) {
            height = (unsigned int) ((long long) *(m->get("height")));
        }
        if (m->has("title", BPTString)) {
            title = (std::string) *(m->get("title"));
        }
    }
}


static void
runLoopCallBack(void * ctx, bp::runloop::Event e)
{
    InstallManager * im = (InstallManager *) e.payload();
    assert(im != NULL);
    im->run();
}

static void
usage()
{
    stringstream ss;
    ss << "usage: BrowserPlusInstaller [-nogui=<anything> [-verbose=<anything>] "
       << "[-pkg=<path>] [-log=<loglevel>] [-logfile=<filename>|console] "
       << "[-appendToLog=<anything>] [-locale=<locale>]";
    BPLOG_ERROR(ss.str());
    cerr << ss.str() << endl;
    exit(-1);
}

static Path
resolvePath(Path pathToBinary,
            Path updatePkg)
{
    Path resolvedPath = updatePkg;
    
    if (!updatePkg.empty() && !boost::filesystem::exists(updatePkg))
    {
        resolvedPath = pathToBinary.parent_path() / updatePkg;
        
        // if that file doesn't exist, then we won't change the updatePkg
        // path, primarily to keep suspicious looking unexpected changes
        // out of the logfile
        if (!boost::filesystem::exists(resolvedPath)) {
            resolvedPath = updatePkg;
        }
    }
    
    return resolvedPath;
}



int
main(int argc, const char** argv)
{
	// on win32, may have non-ascii chars in args.  deal with it
	APT::ARGVConverter conv;
    conv.convert(argc, argv);

    int rval = 0;
    Path exeDir = canonicalPath(Path(argv[0])).parent_path();
    
    // debug logging on be default.  logfile cannot be in same dir
    // as executable since a mounted mac .dmg is read-only
    Path logFile = getTempDirectory() / "BrowserPlusInstaller.log";
    (void) remove(logFile);
    string logLevel = "debug";

    // we must get current user's locale, this may be overridded with the
    // -locale flag.
    string locale = bp::localization::getUsersLocale();

    // the user interaction skin, defaults to GUI but can be set
    // to cmd line with flags.
    shared_ptr<InstallerSkin> skin;

    Path updatePkg;
    string version;
    bool truncateLog = true;

    vector<string> args;
    for (int i = 1; i < argc; i++) {
        // skip args starting with -psn which deliver the "Process Serial
        // Number" and are added by the OSX launcher
        if (!strncmp(argv[i], "-psn", 4)) continue;

        args = bp::strutil::split(argv[i], "=");
        if (!args[0].compare("-logfile")) {
            if (!args[1].compare("console")) {
                logFile.clear();
            } else {
                logFile = Path(args[1]);
            }
        } else if (!args[0].compare("-log")) {
            logLevel = args[1];
        } else if (!args[0].compare("-appendToLog")) {
            truncateLog = false;
        } else if (!args[0].compare("-verbose")) {
			skin.reset(new InstallerSkinVerbose);
        } else if (!args[0].compare("-nogui")) {
			skin.reset(new InstallerSkin);
        } else if (!args[0].compare("-pkg")) {
            updatePkg = Path(args[1]);
            version = versionFromPackage(updatePkg);
            if (version.empty()) {
                usage();
            }
            // handle the case where the path is relative to the binary
            // (YIB-2917492)
            updatePkg = resolvePath(Path(argv[0]), updatePkg);
            if (!boost::filesystem::exists(updatePkg)) {
                BP_THROW("update package " + updatePkg.externalUtf8()
                         + " not found");
            }
        } else if (!args[0].compare("-version")) {
            version = args[1];
        } else if (!args[0].compare("-locale")) {
            locale = args[1];
        } else {
            usage();
        }
    }
    
    // set the appropriate locale for strings generated from the Installer
    Path stringsPath = exeDir / "strings.json";
    Installer::setLocalizedStringsPath(stringsPath, locale);
    
    if (!logFile.empty()) {
        bp::log::setupLogToFile(logFile, logLevel, truncateLog);
    } else if (!logLevel.empty()) {
        bp::log::setupLogToConsole(logLevel);
    }

    BPLOG_INFO_STRM("exeDir = " << exeDir);

    // if skin is NULL, we're in GUI mode.  we'll find the correct
    // UI based on locale, then we'll 
    if (skin == NULL) 
    {
        Path uiDir = exeDir / "ui";
        Path uiPath = bp::localization::getLocalizedUIPath(uiDir, locale);
        if (uiPath.empty()) {
            stringstream ss;
            ss << "Running in GUI mode, no interface found in '"
               << uiDir << "'.  cannot "
               << "continue!";
            BPLOG_ERROR(ss.str());
            cerr << ss.str() << endl;
            exit(1);
        }
        skin.reset(new InstallerSkinGUI(uiPath));        
        BPLOG_INFO("got GUI installer skin");
    }

	Path configPath = exeDir / "installer.config";
	Path keyPath = exeDir / "BrowserPlus.crt";

    Path destDir = getTempPath(getTempDirectory(), "BrowserPlusInstaller");
    (void) remove(destDir);  // doze re-uses same dir.  sigh
    try {
        list<string> servers;
        list<CoreletRequireStatement> services;
        string permissions;
        // default width and height
        unsigned int width = 400, height = 440;
        std::string installerTitle =
            bp::install::Installer::getString(
                bp::install::Installer::kInstallerTitle);
        
        // Dig stuff out of config file.  Command line args for 
        // updatePackage and version take precedence
        Path tmpPkg;
        string tmpVersion;
        string autoUpdatePermissions;
        readConfig(configPath, servers, tmpPkg, tmpVersion,
                   services, permissions, autoUpdatePermissions,
                   width, height, installerTitle);

        // if an updatePkg was specified on the command line, use that
        // (case where !updatePkg.empty())
        if (updatePkg.empty()) {
            updatePkg = tmpPkg;
            version = tmpVersion;
            // handle the case where the path is relative to the
            // binary (YIB-2917492)
            updatePkg = resolvePath(Path(argv[0]), updatePkg);
        }
        BPLOG_INFO_STRM("update package: " << updatePkg);
        BPLOG_INFO_STRM("update version: " << version);

        bp::runloop::RunLoop rl;

        // let's build up the InstallManager object which manages the
        // installation and calls back into the appropriate output skin.
        shared_ptr<InstallManager> im(
            new InstallManager(exeDir, destDir, updatePkg, keyPath,
                               servers, version, services, permissions,
                               autoUpdatePermissions, skin, &rl, width,
                               height, installerTitle));
        
        // now we're ready to start the main runloop
        rl.setCallBacks(runLoopCallBack, NULL);
        rl.init();
        rl.sendEvent(bp::runloop::Event((void *) im.get()));
        rl.run();
        rl.shutdown();

        // Note, we won't get here, InstallerManager will call exit() 
    } catch (const bp::error::Exception& e) {
        BPLOG_ERROR(e.what());
        rval = -1;
    }

    // Note, we will only get here on exceptions.  Otherwiser, 
    // InstallerManager exits
    bp::file::remove(destDir);
    exit(rval);
}
