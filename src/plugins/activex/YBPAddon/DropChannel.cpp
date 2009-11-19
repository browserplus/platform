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

/*
 *  DropChannel.cpp
 *
 *  Represents a connection to an html source element over an existing
 *  ActiveX connection point channel.
 *  
 *  Created by David Grigsby on 11/11/2007.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include "stdafx.h"
#include "DropChannel.h"

#include "AxDropManager.h"
#include "BPUtils/BPLog.h"
#include "HTMLRender/IEUtils_Windows.h"


using namespace std;


DropChannel::DropChannel( int nChanNum, AxDropManager* pDropMgr ) :
    DropTargetBase(),
    m_nChanNum( nChanNum ),
    m_pDropMgr( pDropMgr )
{

}

DropChannel::DropChannel( const DropChannel& rhs ) :
    DropTargetBase(rhs),
    m_nChanNum( rhs.m_nChanNum ),
    m_pDropMgr( rhs.m_pDropMgr )
{

}


DropChannel::~DropChannel()
{

}


bool DropChannel::isAvailable()
{
    return m_elemSource == 0;
}


std::string DropChannel::sourceId()
{
    if (!m_elemSource)
    {
        return "";
    }
    
    CComQIPtr<IHTMLElement> htElem( m_elemSource );
    CComBSTR bsId;
    htElem->get_id( &bsId );
    return string( _bstr_t( bsId ) );
}


bool DropChannel::connect( const std::string& sElementId,
                           const std::set<std::string>& mimeTypes,
                           bool includeGestureInfo,
                           unsigned int limit)
{
    if (!isAvailable())
    {
        BPLOG_ERROR( "Attempted to connect to channel in use!" );
        return false;
    }

    CComPtr<IHTMLElement> elem;
    if (!bp::ie::getDocElementById( m_pDropMgr->browser(), sElementId,
                                    elem ))
    {
        BPLOG_ERROR( "getDocElementById failed!" );
        return false;
    }

    m_elemSource = elem;

    // We need the Drop Manager's help to actually wire up the channel.
    if (!m_pDropMgr->connectDropChannel( m_nChanNum, m_elemSource ))
    {
        BPLOG_ERROR( "connectDropChannel failed!" );
        m_elemSource = 0;
        return false;
    }

    // set base class gunk
    m_mimetypes = mimeTypes;
    m_includeGestureInfo = includeGestureInfo;
    m_limit = limit;
    
    return true;
}


bool DropChannel::disconnect()
{
    if (!m_elemSource)
    {
        return true;
    }

    // We need the Drop Manager's help to actually disconnect the channel.
    if (!m_pDropMgr->disconnectDropChannel( m_nChanNum, m_elemSource ))
    {
        BPLOG_ERROR( "disconnectDropChannel failed!" );
        return false;
    }

    m_elemSource = 0;
    // reset base class gunk
    m_mimetypes.clear();
    m_includeGestureInfo = false;
    m_limit = 0;
    
    return true;
}
