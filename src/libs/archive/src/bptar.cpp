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
 * bptar - an abstraction around bsd licensed tarfile creation and
 *         extraction functionality provided by libarchive.
 */


#define LIBARCHIVE_STATIC

#include "bptar.h"
#include "BPUtils/bpfile.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bperrorutil.h"
#include "libarchive/archive.h"
#include "libarchive/archive_entry.h"

#include <fstream>
#include <string.h>

#define BP_TAR_BUF_SIZE (1024 * 64)

using namespace bp::tar;
namespace bfs = boost::filesystem;

Extract::Extract()
    : m_state(NULL)
{
}
    
Extract::~Extract()
{
    this->close();
}

bool
Extract::load(const std::string& tarData)
{
    if (m_state != NULL) return false;
    m_data = tarData;
    return init();
}

bool
Extract::open(const bp::file::Path& tarFile)
{
    if (m_state != NULL) return false;

    if (!bp::strutil::loadFromFile(tarFile, m_data)) {
        return false;
    }

    return init();
}

bool
Extract::init()
{
    struct archive *a = archive_read_new();
    BPASSERT(a != NULL);
    if (archive_read_support_compression_none(a) ||
        archive_read_support_format_gnutar(a) || 
        archive_read_support_format_tar(a) || 
        archive_read_open_memory(a, (void *) m_data.c_str(), m_data.length()))
    {
        archive_read_finish(a);
        return false;
    }
    m_state = (void *) a;
    return true;
}

bool
Extract::close()
{
    if (m_state) {
        archive_read_finish((struct archive *) m_state);
        m_state = NULL;
    }
    m_data.clear();
    return true;
}

std::vector<bp::file::Path>
Extract::enumerateContents()
{
    std::vector<bp::file::Path> contents;

    struct archive * a = (struct archive *) m_state;
    struct archive_entry * ae = NULL;

    if (a == NULL) return contents;
    
    while (!(archive_read_next_header(a, &ae))) {
        const char * pn = archive_entry_pathname(ae);

        // and what if it IS null?
        if (pn != NULL && strlen(pn) > 0) {
            contents.push_back(bp::file::Path(pn));
        }
    }

    // now we'll close and re-open the archive with touching m_data
    archive_read_finish((struct archive *) m_state);
    m_state = NULL;
    init();
    
    return contents;
}

bool
Extract::extractSingle(const bp::file::Path& itemName,
                       std::ostream & os)
{
    unsigned char buf[BP_TAR_BUF_SIZE];
    bool success = false;

    struct archive * a = (struct archive *) m_state;;
    struct archive_entry * ae = NULL;

    if (a == NULL) return false;
    
    while (!(archive_read_next_header(a, &ae))) {
        const char * pn = archive_entry_pathname(ae);

        // and what if it IS null?
        if (pn != NULL && strlen(pn) > 0 &&
            !itemName.utf8().compare(std::string(pn)))
        {
            // key off the tar header for file type
            unsigned int type = archive_entry_filetype(ae);

            if (AE_IFREG == type) {
                size_t sz;
                while ((sz = archive_read_data(a, (void *) buf,
                                               BP_TAR_BUF_SIZE)) > 0)
                {
                    os.write((const char *) buf, sz);
                }
                success = true;
            } else {
                // I don't know how to extract entries other than
                // regular files
            }
            break;
        }
    }

    // now we'll close and re-open the archive with touching m_data
    archive_read_finish((struct archive *) m_state);
    m_state = NULL;
    init();
    
    return success;
}

bool
Extract::extract(const bp::file::Path& destDir)
{
    // TODO: add progress extraction progress
    
    unsigned char buf[BP_TAR_BUF_SIZE];

    struct archive * a = (struct archive *) m_state;;
    struct archive_entry * ae = NULL;

    if (a == NULL) return false;
    
    while (!(archive_read_next_header(a, &ae))) {
        const char * pn = archive_entry_pathname(ae);

        // and what if it IS null?
        if (pn != NULL && strlen(pn) > 0) {
            bp::file::Path path = destDir / pn;

            // key off the tar header for file type
            unsigned int type = archive_entry_filetype(ae);
            unsigned int mode = archive_entry_mode(ae);

            if (AE_IFDIR == type) {
                try {
                    bfs::create_directories(path);
                } catch(const bp::file::tFileSystemError&) {
                    return false;
                }
            } else if (AE_IFREG == type) {
                std::ofstream fstream;
                // now let's write this entry to disk
                if (!bp::file::openWritableStream(fstream, path,
                                                  std::ios::binary))
                {
                    return false;
                }
                size_t sz;
                while ((sz = archive_read_data(a, (void *) buf,
                                               BP_TAR_BUF_SIZE)) > 0)
                {
                    fstream.write((const char *) buf, sz);
                }
                fstream.close();

                // now let's set times and perms
                bp::file::FileInfo fi;
                fi.mode = mode;
                fi.ctime = archive_entry_ctime(ae);
                fi.atime = archive_entry_atime(ae); 
                fi.mtime = archive_entry_mtime(ae);
                bp::file::setFileProperties(path, fi);
            } else {
                // what other file types must we support!?
                return false;
            }
        }
    }
    
    return true;
}


Create::Create()
    : m_state(NULL)
{
}
    
Create::~Create()
{
    (void) close();
}

static
int writeOpenCallback(struct archive *, void *_client_data)
{
    // noop
    return 0;
}

int
Create::writeCloseCallback(void *, bp::tar::Create * cobj)
{
    cobj->m_file.close();
    return 0;
}

long
Create::writeCallback(void *,  bp::tar::Create * cobj,
                      const void * buf, size_t len)
{
    long before = cobj->m_file.tellp();
    cobj->m_file.write((const char *) buf, len);
    long after = ((long) cobj->m_file.tellp() - before);
    return after;
}


bool
Create::open(const bp::file::Path& tarFile)
{
    if (m_state != NULL) return false;

    // let's try to open our output stream
    if (!bp::file::openWritableStream(m_file, tarFile, std::ios::binary))
    {
        return false;
    }

    struct archive *a = archive_write_new();    
    BPASSERT(a != NULL);
    if (archive_write_set_compression_none(a) ||
        archive_write_set_format_ustar(a) || 
        archive_write_open(a, (void *) this, writeOpenCallback,
                           (archive_write_callback *) writeCallback,
                           (archive_close_callback *) writeCloseCallback))
    {
        archive_write_close(a);
        archive_write_finish(a);
        return false;
    }

    m_state = (void *) a;
    return true;
}

bool
Create::close()
{
    if (m_state != NULL) {
        struct archive *a = (struct archive *) m_state;
        archive_write_close(a);
        archive_write_finish(a);
        m_state = NULL;
        m_file.close();
		m_file.clear();
        return true;
    }
    return false;
}

bool
Create::addFile(const bp::file::Path& fileToAdd,
                const bp::file::Path& fileNameInTarIn)
{
    if (m_state == NULL) return false;
    struct archive *a = (struct archive *) m_state;
    bool isDir = false;

    // get information about the file on disk;  
    bp::file::FileInfo fi;
    if (!bp::file::statFile(fileToAdd, fi)) {
        return false;
    }

    // write header
    struct archive_entry * ae = archive_entry_new();
    archive_entry_clear(ae);

    // now include file information
    archive_entry_set_atime(ae, fi.atime, 0);
    archive_entry_set_mtime(ae, fi.mtime, 0);
    archive_entry_set_ctime(ae, fi.ctime, 0);
    archive_entry_set_mode(ae, static_cast<unsigned short>(fi.mode));

    bp::file::Path fileNameInTar = fileNameInTarIn;
    if (bp::file::isDirectory(fileToAdd)) {
        // append a trailing pathsep to directories, this seems to be
        // a convention
        std::string s = fileNameInTar.utf8();
        if (s[s.length()-1] != '/') {
            s.append("/");
            fileNameInTar = s;
        }
        archive_entry_set_filetype(ae, AE_IFDIR);
        isDir = true;
    }  else if (!bp::file::isOther(fileToAdd)) {
        archive_entry_set_size(ae, bp::file::size(fileToAdd));
        archive_entry_set_filetype(ae, AE_IFREG);
    }

    archive_entry_set_pathname(ae, fileNameInTar.utf8().c_str());

    
	int rv = archive_write_header(a, ae);
    if (0 != rv) {
        archive_entry_free(ae);
        return false;
    }
    archive_entry_free(ae);

    // now write data for files
    if (!isDir) {
        unsigned char buf[BP_TAR_BUF_SIZE];
        std::ifstream fstream;
        // now let's write this entry to disk
        if (!bp::file::openReadableStream(fstream, fileToAdd, std::ios::binary))
        {
            return false;
        }

        for (;;)
        {
            fstream.read((char *) buf, BP_TAR_BUF_SIZE);
            size_t rd = fstream.gcount(); 
			if (rd > 0) {
                long wt = archive_write_data(a, (void *) buf, fstream.gcount());
                if (wt != fstream.gcount()) {
                    return false;
                }
            }

            if (fstream.eof()) break;
            if (fstream.fail()) return false;
        }
    }
    
    return true;
}


