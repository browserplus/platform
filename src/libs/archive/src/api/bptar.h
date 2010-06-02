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

/**
 * bptar.h - an abstraction around bsd licensed tarfile creation and
 *           extraction functionality provided by libarchive.
 */

#ifndef __BPTAR_H__
#define __BPTAR_H__

#include <string>
#include <vector>
#include <fstream>
#include <BPUtils/bpfile.h>

namespace bp { namespace tar {

class Extract 
{
  public:
    Extract();
    ~Extract();    
    // open a tarfile
    bool open(const bp::file::Path& tarFile);
    // load a tarfile that's already in memory
    bool load(const std::string& tarData);
    // list all contents of a tarfile (a vector of relative path strings
    // is returned)
    std::vector<bp::file::Path> enumerateContents();
    // extract a single element of a tarfile to a stream,
    // itemName is the name of the element in the tarfile
    bool extractSingle(const bp::file::Path& itemName,
                       std::ostream & os);
    // close a tarfile
    bool close();    
    // extract all contents to a named destination directory
    bool extract(const bp::file::Path& destDir);

    
  private:
    // once m_data is populated with the tarr'd stream, initialize the
    // tar extraction
    bool init();

    // void * pointer to shield client from libarchive headers
    void * m_state;

    // file data - we read the whole file into memory
    std::string m_data;
};

class Create
{
  public:
    Create();
    ~Create();    
    // create a new tarfile
    bool open(const bp::file::Path& tarFile);
    // close a tarfile
    bool close();    
    // add a file to the tarball.  fileToAdd should be an absolute path,
    // or relative from the pwd.  fileNameInTar is the relative pathing
    // that will appear in the tar file and needen't have any relation to
    // "fileToAdd".
    //
    // returns false on failure, true on success.
    bool addFile(const bp::file::Path& fileToAdd,
                 const bp::file::Path& fileNameInTar);

  private:
    static int writeCloseCallback(void *, bp::tar::Create * cobj);

    static long writeCallback(void *, bp::tar::Create * cobj,
                              const void * buf, size_t len);

    // void * pointer to shield client from libarchive headers
    void * m_state;
    std::ofstream m_file;
};

}; };

#endif
