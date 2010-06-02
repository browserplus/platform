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
 *  DropChannel.h
 *
 *  Represents a connection to an html source element over an existing
 *  ActiveX connection point channel.
 *  
 *  Created by David Grigsby on 11/11/2007.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef __DROPCHANNEL_H__
#define __DROPCHANNEL_H__

#include "PluginCommonLib/DropTargetBase.h"


// Forward Decls
class AxDropManager;


class DropChannel : public virtual DropTargetBase
{
public:
    DropChannel( int nChanNum, AxDropManager* pDropMgr );
    DropChannel( const DropChannel& );
    ~DropChannel();

    bool isAvailable();
    std::string sourceId();

    bool connect( const std::string& sSourceId,
                  const std::set<std::string>& mimeTypes,
                  bool includeGestureInfo,
                  unsigned int limit );
    bool connect( const std::string& sSourceId,
                  const std::string& sVersion );
    bool disconnect();

private:
    int m_nChanNum;
    AxDropManager* m_pDropMgr;

    CComPtr<IHTMLTextContainer> m_elemSource;
    bool m_enabled;

//  DropChannel& operator=( const DropChannel& );
};


#endif // __DROPCHANNEL_H__
