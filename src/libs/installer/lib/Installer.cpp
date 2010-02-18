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

#include "BPUtils/BPUtils.h"
#include "Permissions/Permissions.h"
#include "api/Installer.h"
#include "api/Utils.h"
#include <memory.h>
#include <fcntl.h>

#ifdef WIN32
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#else
#include <unistd.h>
#endif

using namespace std;
using namespace std::tr1;
using namespace bp::paths;
using namespace bp::error;
using namespace bp::install;
namespace bpf = bp::file;
namespace bfs = boost::filesystem;

// Localization keys
//
const char* Installer::kProductNameShort = "kProductNameShort";
const char* Installer::kInstallerTitle = "kInstallerTitle";
const char* Installer::kNotAdmin = "kNotAdmin"; 
const char* Installer::kInstallerAlreadyRunning = "kInstallerAlreadyRunning";
const char* Installer::kStartingInstallation = "kStartingInstallation";
const char* Installer::kCopyingDaemonBits = "kCopyingDaemonBits";
const char* Installer::kUpdatingPermissions = "kUpdatingPermissions";
const char* Installer::kInstallingPlugins = "kInstallingPlugins";
const char* Installer::kInstallingPreferencePane = "kInstallingPreferencePane";
const char* Installer::kInstallingBundledServices = "kInstallingBundledServices";
const char* Installer::kInstallingUninstaller = "kInstallingUninstaller";
const char* Installer::kFinishingUp = "kFinishingUp";
const char* Installer::kErrorEncountered = "kErrorEncountered";
const char* Installer::kConfigLink = "kConfigLink";
const char* Installer::kUninstallLink = "kUninstallLink";
const char* Installer::kPlatformDownloading = "kPlatformDownloading";
const char* Installer::kServicesDownloading = "kServicesDownloading";
const char* Installer::kNewerVersionInstalled = "kNewerVersionInstalled";

string Installer::s_locale;
bpf::Path Installer::s_stringsPath;

Installer::Installer(const bpf::Path& dir,
                     bool deleteWhenDone)
    : m_dir(dir), m_deleteWhenDone(deleteWhenDone)
{
    string versionString = bpf::utf8FromNative(m_dir.filename());
    if (!m_version.parse(versionString)) {
        BP_THROW("Bad version: " + versionString);
    }

    bp::paths::createDirectories(m_version.majorVer(),
                                 m_version.minorVer(),
                                 m_version.microVer());

    bpf::Path platInfo = m_dir / "platformInfo.json";
    utils::readPlatformInfo(platInfo);
}


Installer::~Installer()
{
    if (m_deleteWhenDone) {
        remove(m_dir);
    }
}


void
Installer::run()
{
    BPLOG_INFO_STRM("Begin install of version " << m_version.asString()
                    << " onto platform " << bp::os::PlatformAsString()
                    << ", version " << bp::os::PlatformVersion());

    sendProgress(0);
    
    bpf::Path installingPath;
    try {
        // Make sure that a newer version (within same major rev)
        // isn't installed
        vector<bp::ServiceVersion> plats = utils::installedVersions();
        vector<bp::ServiceVersion>::const_iterator iter;
        for (iter = plats.begin(); iter != plats.end(); ++iter) {
            if (iter->majorVer() != m_version.majorVer()) {
                continue;
            }
            if (iter->compare(m_version) > 0) {
                BP_THROW(getString(kNewerVersionInstalled));
            }
        }

        // write .installing file, keeps daemon from cleaning us up while
        // we're in the middle of an install
        installingPath = getBPInstallingPath(m_version.majorVer(),
                                             m_version.minorVer(),
                                             m_version.microVer());
        BPTime now;
        if (!bp::strutil::storeToFile(installingPath, now.asString())) {
            BP_THROW(lastErrorString("unable to write " + installingPath.externalUtf8()));
        }

        sendProgress(5);
        sendStatus(getString(kStartingInstallation));    
        preflight();
        sendProgress(10);
        sendStatus(getString(kCopyingDaemonBits));
        installDaemon();
        sendProgress(20);
        sendStatus(getString(kUpdatingPermissions));
        installPermissions();
        sendProgress(30);
        sendStatus(getString(kInstallingPlugins));
        installPlugins();
        sendProgress(40);
        sendStatus(getString(kInstallingPreferencePane));
        installPrefPanel();
        sendProgress(50);
        sendStatus(getString(kInstallingBundledServices));
        installServices();
        sendProgress(60);
        sendStatus(getString(kInstallingUninstaller));
        installUninstaller();
        sendProgress(70);
        makeLinks();
        sendProgress(80);
        sendStatus(getString(kFinishingUp));
        postflight();
        sendProgress(90);
        allDone();
        sendProgress(95);
    } catch (const bp::error::Exception& e) {
        BPLOG_ERROR_STRM("Installer.run() catches " << e.what());
        sendError(e.what());
    }
    (void) remove(installingPath);
    sendProgress(100);
    sendDone();
}


void
Installer::installDaemon()
{
    BPLOG_DEBUG("begin Installer::installDaemon");

    // Now update platform
    bpf::Path productDir = getProductDirectory(m_version.majorVer(),
                                               m_version.minorVer(),
                                               m_version.microVer());

    // Now copy in platform.  We try to remove an existing one first.
    // Then we use doCopy() to do the copy, which will "accept"
    // a failure as long as the src and dst files are identical.
    // This happens if windows has a file open (like the plugin).
    bpf::Path daemonDir = m_dir / "daemon";
    if (!remove(productDir)) {
        BPLOG_DEBUG_STRM("unable to delete " << productDir);
    }
    doCopy(daemonDir, productDir);  // throws on failure

    // Create copy of BrowserPlusCore as BrowserPlusService.
    // Can use hard link on unix
    bpf::Path daemon = productDir / "BrowserPlusCore";
    daemon = canonicalProgramPath(daemon);
    bpf::Path svc = productDir / "BrowserPlusService";
    svc = canonicalProgramPath(svc);
#ifdef WIN32
    doCopy(daemon, svc);
#else
    try {
        bfs::create_symlink(daemon, svc);
    } catch(const bpf::tFileSystemError& e) {
        BP_THROW("unable to create " + svc.externalUtf8()
                 + ": " + e.what());
    }
#endif
    BPLOG_DEBUG("complete Installer::installDaemon");
}    
    

void
Installer::installPermissions()
{
    BPLOG_DEBUG("begin Installer::installPermissions");

    // Install new public keys if needed.  They are needed if
    // they are not found in the current keys.
    bpf::Path newCertPath = m_dir / "permissions" / "BrowserPlus.crt";
    if (bpf::exists(newCertPath)) {
        bool needNewKeys = true;
        string newKeys;
        if (!bp::strutil::loadFromFile(newCertPath, newKeys)) {
            BP_THROW(lastErrorString("unable to load keys from "
                                     + newCertPath.externalUtf8()));
        }
    
        string curKeys;
        bpf::Path certPath = getCertFilePath();
        if (bpf::exists(certPath)) {
            (void) bp::strutil::loadFromFile(certPath, curKeys);
            needNewKeys = (curKeys.find(newKeys) == string::npos);
        }
        if (needNewKeys) {
            curKeys.append(newKeys);
            if (!bp::strutil::storeToFile(certPath, curKeys)) {
                BP_THROW(lastErrorString("unable to store keys to "
                                         + certPath.externalUtf8()));
            }
        }
    }
    
    PermissionsManager* pmgr = PermissionsManager::get(m_version.majorVer(),
                                                       m_version.minorVer(),
                                                       m_version.microVer());

    // Now for domain permissions
    vector<bpf::Path> files;
    files.push_back(bpf::Path("updateDomainPermissions"));
    files.push_back(bpf::Path("configDomainPermissions"));
    for (unsigned int i = 0; i < files.size(); i++) {
        bpf::Path path = m_dir / "permissions" / files[i];
        string json;
        if (!bpf::exists(path)) {
            continue;
        }
        if (!bp::strutil::loadFromFile(path, json)) {
            BPLOG_WARN_STRM(lastErrorString("unable to load permissions from "
                                            + path.externalUtf8()));
            continue;
        }
        bp::Map* m = dynamic_cast<bp::Map*>(bp::Object::fromPlainJsonString(json));
        if (!m) {
            BPLOG_WARN_STRM("bad permissions format: " << json);
            continue;
        }
        
        // Map keys are domain, values are list of permissions to grant
        // e.g.
        // {
        //    "browserplus.org": ["perm1, "perm2"]
        // }
        bp::Map::Iterator domainIter(*m);
        const char* domain = NULL;
        while ((domain = domainIter.nextKey()) != NULL) {
            // don't use Map::getList(), it will interpret any / chars in key!
            const bp::List* perms = dynamic_cast<const bp::List*>(m->value(domain));
            if (!perms) {
                BPLOG_WARN_STRM("bad permissions format: " << domain);
                continue;
            }
            for (unsigned int i = 0; i < perms->size(); i++) {
                const bp::String* s = dynamic_cast<const bp::String*>(perms->value(i));
                if (!s) {
                    BPLOG_WARN_STRM("bad permissions format: " << domain);
                    continue;
                }
                pmgr->addDomainPermission(domain, s->value());
            }
        }
    }

    try {
        // Now for auto-update permissions
        bpf::Path path = m_dir / "permissions" / "configAutoUpdatePermissions";
        if (bpf::exists(path)) {
            string json;
            if (!bp::strutil::loadFromFile(path, json)) {
                BP_THROW(lastErrorString("unable to load permissions from "
                                         + path.externalUtf8()));
            }
            bp::Map* m = dynamic_cast<bp::Map*>(bp::Object::fromPlainJsonString(json));
            if (!m) {
                BP_THROW("bad permissions format: " + json);
            }
            
            // Map keys are domain, values map giving platform/service autoupate permission
            // e.g.
            // {
            //    "browserplus.org": {
            //         "platform": true,
            //         "services": {
            //             "Uploader": true,
            //             "Zipper": true
            //         }
            //    }
            // }
            bp::Map::Iterator domainIter(*m);
            const char* domain = NULL;
            while ((domain = domainIter.nextKey()) != NULL) {
                // don't use Map::getMap(), it will interpret any / chars in key!
                const bp::Map* perms = dynamic_cast<const bp::Map*>(m->value(domain));
                if (!perms) {
                    BPLOG_WARN_STRM("bad autoupdate permissions format: " << domain);
                    continue;
                }
                bool allowed;
                if (perms->getBool("platform", allowed)) {
                    pmgr->setAutoUpdatePlatform(domain,
                                                allowed ? PermissionsManager::eAllowed
                                                : PermissionsManager::eNotAllowed);
                }
                const bp::Map* serviceMap = NULL;
                if (perms->getMap("services", serviceMap)) {
                    bp::Map::Iterator serviceIter(*serviceMap);
                    const char* service = NULL;
                    while ((service = serviceIter.nextKey()) != NULL) {
                        if (serviceMap->getBool(service, allowed)) {
                            pmgr->setAutoUpdateService(domain, service,
                                                       allowed ? PermissionsManager::eAllowed
                                                       : PermissionsManager::eNotAllowed);
                        } else {
                            BPLOG_WARN_STRM("bad autoupdate permissions format: " << domain);
                        }
                    }
                }
            }
        }
    } catch (const bp::error::Exception& e) {
        BPLOG_WARN(e.what());
    }
    
    BPLOG_DEBUG("complete Installer::installPermissions");
}


void
Installer::installServices()
{
    BPLOG_DEBUG("begin Installer::installServices");

    bpf::Path servicesDir = m_dir / "services";
    if (!bfs::is_directory(servicesDir)) {
        BPLOG_DEBUG_STRM(servicesDir << " does not exist, no services installed");
        return;
    }
    try {
        bpf::tDirIter sit_end;
        for (bpf::tDirIter sit(servicesDir); sit != sit_end; ++sit) {
            bpf::Path service = sit->path().filename();
            try {
                bpf::tDirIter vit_end;
                for (bpf::tDirIter vit(sit->path()); vit != vit_end; ++vit) {
                    bpf::Path version = vit->path().filename();
                    bpf::Path source = sit->path();
                    bpf::Path dest = getCoreletDirectory() / service / version;
                    try {
                        bfs::create_directories(dest.parent_path());
                    } catch(const bpf::tFileSystemError& e) {
                        BPLOG_WARN_STRM("unable to create " << dest
                                        << ": " << e.what());
                        continue;
                    }
                    (void) remove(dest);
                    try {
                        doCopy(source, dest.parent_path());
                    } catch (const bp::error::Exception& e) {
                        BPLOG_WARN(e.what());
                        continue;
                    }
                }
            }
            catch (const bpf::tFileSystemError& e) {
                BPLOG_WARN_STRM("unable to iterate thru " << sit->path()   
                                << ": " << e.what());
            }
        }
    } catch (const bpf::tFileSystemError& e) {
        BPLOG_WARN_STRM("unable to iterate thru " << servicesDir   
                        << ": " << e.what());
    }
    
    BPLOG_DEBUG("complete Installer::installServices");
}


void
Installer::installUninstaller()
{
    BPLOG_DEBUG("begin Installer::installUninstaller");
    bpf::Path dest = getUninstallerPath();
    bpf::Path src = m_dir / dest.filename();
    try {
        bfs::create_directories(dest.parent_path());
    } catch(const bpf::tFileSystemError&) {
        BP_THROW(lastErrorString("unable to create "
            + bpf::Path(dest.parent_path()).externalUtf8()));
    }
    (void) remove(dest);
    doCopy(src, dest);
#ifdef WIN32
    // uninstall icon
    src = m_dir / "ybang.ico";
    dest = dest.parent_path() / "ybang.ico";
    (void) remove(dest);
    doCopy(src, dest);
#endif
#ifdef MAC
    // uninstaller has moved from ~/Applications, kill old one
    bpf::Path ydir = bpf::Path(getenv("HOME")) / "Applications" / "Yahoo!";
    bpf::Path s = ydir / "BrowserPlus";
    (void) remove(s);
    if (bfs::is_directory(ydir) && bfs::is_empty(ydir)) {
        (void) remove(ydir);
    }
#endif
    BPLOG_DEBUG("complete Installer::installUninstaller");
}


void 
Installer::allDone()
{
    BPLOG_DEBUG("begin Installer::allDone");
    bpf::Path installedPath = getBPInstalledPath(m_version.majorVer(),
                                                 m_version.minorVer(),
                                                 m_version.microVer());
    BPTime now;
    if (!bp::strutil::storeToFile(installedPath, now.asString())) {
        BP_THROW(lastErrorString("unable to write " + installedPath.externalUtf8()));
    }
    BPLOG_DEBUG("complete Installer::allDone");
}


void
Installer::sendStatus(const string& s)
{
    BPLOG_DEBUG_STRM("sendStatus " << s);
    shared_ptr<IInstallerListener> l = m_listener.lock();
    if (l) {
        l->onStatus(s);
    }
}


void
Installer::sendProgress(unsigned int pct)
{
    BPLOG_DEBUG_STRM("sendProgress " << pct);
    shared_ptr<IInstallerListener> l = m_listener.lock();
    if (l) {
        l->onProgress(pct);
    }
}


void
Installer::sendError(const string& s)
{
    BPLOG_DEBUG_STRM("sendError " << s);
    shared_ptr<IInstallerListener> l = m_listener.lock();
    if (l) {
        l->onError(s);
    }
}


void
Installer::sendDone()
{
    BPLOG_DEBUG_STRM("sendDone")
    shared_ptr<IInstallerListener> l = m_listener.lock();
    if (l) {
        l->onDone();
    }
}


string 
Installer::getString(const char* key)
{
    string rval;
    (void) bp::localization::getLocalizedString(key, s_locale, rval,
                                                s_stringsPath);
    return rval;
}


void
Installer::doCopy(const bpf::Path& src,
                  const bpf::Path& dest)
{
    BPLOG_DEBUG_STRM("doCopy: attempt to copy " << src
                     << " to " << dest);
    if (!bpf::exists(src)) {
        BP_THROW(lastErrorString(src.externalUtf8() + " does not exist"));
    }
    
    if (bfs::is_directory(src)) {
        try {
            bfs::create_directories(dest);
        } catch(const bpf::tFileSystemError&) {
            BP_THROW(lastErrorString("unable to create " + dest.externalUtf8()));
        }
        try {
            bpf::tRecursiveDirIter end;
            for (bpf::tRecursiveDirIter it(src); it != end; ++it) {
                bpf::Path srcPath = bpf::Path(it->path());
                bpf::Path relPath = srcPath.relativeTo(src);
                bpf::Path destPath = dest / relPath;
                if (bfs::is_regular(srcPath)) {
                    doSingleFileCopy(srcPath, destPath);
                }
            }
        } catch (const bpf::tFileSystemError& e) {
            BP_THROW("unable to iterate thru " + src.externalUtf8()
                     + ": " + e.what());
        }
    } else {
        doSingleFileCopy(src, dest);
    }
    BPLOG_DEBUG_STRM("doCopy: copied " << src
                     << " to " << dest);
}


void
Installer::doSingleFileCopy(const bpf::Path& src,
                            const bpf::Path& dest)
{
    BPLOG_DEBUG_STRM("doSingleFileCopy(" << src << ", " << dest << ")");

    if (!bpf::exists(src)) {
        BP_THROW(lastErrorString(src.externalUtf8() + " does not exist"));
    }
    bpf::Path destParent(dest.parent_path());
    try {
        bfs::create_directories(destParent);
    } catch(const bpf::tFileSystemError&) {
        BP_THROW(lastErrorString("unable to create " + destParent.externalUtf8()));
    }
    
    bpf::Path realDest = dest;
    if (bfs::is_directory(dest)) {
        realDest /= src.filename();
    }

    // first delete final dest.  copy() won't overwrite
    if (remove(realDest)) {
        // now copy source kid to dest
        BPLOG_DEBUG_STRM("copy " << src << " to " << realDest);
        if (!copy(src, realDest)) {
            BP_THROW(lastErrorString("unable to copy " + src.externalUtf8() 
                                     + " to " + realDest.externalUtf8()));
        }
    } else {
        // Hrm, can't delete destination.  That's ok if
        // it's identical to the source.
        string msg = "unable to copy " + src.externalUtf8()
            + " -> " + realDest.externalUtf8();
        if (filesAreIdentical(src, realDest)) {
            BPLOG_DEBUG_STRM(msg + ", but files are identical");
        } else {
            BP_THROW(msg + ", and files are not identical");
        }
    }
    BPLOG_DEBUG_STRM("copied " << src << " to " << realDest);
}


bool
Installer::filesAreIdentical(const bpf::Path& f1,
                             const bpf::Path& f2)
{
    bool rval = false;

    int f1Size = bfs::is_regular_file(f1) ? (int) bfs::file_size(f1) : 0;
    int f2Size = bfs::is_regular_file(f2) ? (int) bfs::file_size(f2) : 0;
    if (f1Size != f2Size) {
        return false;
    }

    // read files into buffers, compare the buffers
    //
    int fd1 = -1, fd2 = -1;
    bpf::tString f1Ext = f1.external_file_string();
    bpf::tString f2Ext = f2.external_file_string();
    unsigned char* buf1 = NULL;
    unsigned char* buf2 = NULL;
    try {
#ifdef WIN32
        // of course windows renamed things
#define read _read
#define close _close

        if (::_wsopen_s(&fd1, f1Ext.c_str(), _O_BINARY | _O_RDONLY,
                        _SH_DENYNO, _S_IREAD) != 0) {
            BP_THROW("unable to open " + f1.externalUtf8());
        }
        if (::_wsopen_s(&fd2, f2Ext.c_str(), _O_BINARY | _O_RDONLY,
                        _SH_DENYNO, _S_IREAD) != 0) {
            BP_THROW("unable to open " + f2.externalUtf8());
        }
#else
        fd1 = ::open(f1Ext.c_str(), O_RDONLY);
        if (fd1 < 0) {
            BP_THROW("unable to open " + f1.externalUtf8());
        }
        fd2 = ::open(f2Ext.c_str(), O_RDONLY);
        if (fd2 < 0) {
            BP_THROW("unable to open " + f2.externalUtf8());
        }
#endif
        buf1 = new unsigned char[f1Size];
        if (buf1 == NULL) {
            BP_THROW("unable to allocate buffer for " + f1.externalUtf8());
        }
        buf2 = new unsigned char[f2Size];
        if (buf2 == NULL) {
            BP_THROW("unable to allocate buffer for " + f2.externalUtf8());
        }
        if (::read(fd1, buf1, f1Size) != f1Size) {
            BP_THROW("unable to read " + f1.externalUtf8());
        }
        if (::read(fd2, buf2, f2Size) != f2Size) {
            BP_THROW("unable to read " + f2.externalUtf8());
        }
        rval = ::memcmp(buf1, buf2, f1Size) == 0;
    } catch (const bp::error::Exception& e) {
        BPLOG_WARN(e.what());
        rval = false;
    }
    if (fd1 >= 0) ::close(fd1);
    if (fd2 >= 0) ::close(fd2);
    if (buf1) delete [] buf1;
    if (buf2) delete [] buf2;
    return rval;
}


void
Installer::removePlatform(const bp::ServiceVersion& version)
{
    // "uninstall" platform
    bpf::Path installedPath = getBPInstalledPath(version.majorVer(),
                                                 version.minorVer(),
                                                 version.microVer());
    BPLOG_DEBUG_STRM("removePlatform " << version.asString()
                     << " tries to remove " << installedPath);
    (void) bpf::remove(installedPath);

    // Make sure that plugins aren't found by browsers
    disablePlugins(version);

    // Now remove platform if possible.  If it can't be removed,
    // daemons will keep trying to remove it when they shutdown.
    // Eventually we'll get the cruft removed.
    bp::platformutil::removePlatform(version, false);
}
