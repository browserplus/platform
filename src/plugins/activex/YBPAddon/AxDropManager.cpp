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

/*
 *  AxDropManager.cpp
 *
 *  ActiveX Drag and Drop manager.
 *
 *  Created by David Grigsby on 11/11/2007.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#include "stdafx.h"
#include "AxDropManager.h"
#include "BPUtils/BPLog.h"
#include "BPUtils/bpstrutil.h"
#include "BPUtils/bpfile.h"
#include "DropChannel.h"
#include "HTMLRender/ComUtils_Windows.h"
#include "HTMLRender/IEUtils_Windows.h"
#include "PluginCommonLib/IDropListener.h"

using namespace std;


AxDropManager::AxDropManager()
{
    // Setup our drop channel vector (fixed size).
    for (int i=0; i<AX_NUM_DROP_CHANNELS; ++i)
    {
        m_vDropChannels.push_back( DropChannel( i, this ) );
    }
}


AxDropManager::~AxDropManager()
{

}



////////////////////////////////////////////////////////////
// IDropManager Methods

bool AxDropManager::addTarget( const std::string& sElement,
                               const std::set<std::string>& mimeTypes,
                               bool includeGestureInfo,
                               unsigned int limit )
{
    // Find an available drop channel.
    tDropChannelVecIt itChan =
        find_if( m_vDropChannels.begin(), m_vDropChannels.end(),
                 mem_fun_ref(&DropChannel::isAvailable) );
    if (itChan == m_vDropChannels.end())
    {
        BPLOG_ERROR( "Unable to add target - no available channels!" );
        return false;
    }
    
    // Connect to the specified dom element over this channel.
    if (!itChan->connect( sElement, mimeTypes, 
                          includeGestureInfo, limit ))
    {
        BPLOG_ERROR( "itChan->connect failed!" );
        return false;
    }

    return true;
}


bool AxDropManager::removeTarget( const std::string& sElement )
{
    // Find the channel using the specified element.
    tDropChannelVecIt itChan = findDropChannel( sElement );
    if (itChan == m_vDropChannels.end())
    {
        BPLOG_ERROR( "Unable to remove target - element name not found!" );
        return false;
    }

    // Disconnect the channel from the element.
    if (!itChan->disconnect())
    {
        BPLOG_ERROR( "itChan->disconnect failed!" );
        return false;
    }

    return true;
}

bool AxDropManager::enableTarget(const std::string& sElement, bool bEnable)
{
    tDropChannelVecIt it = findDropChannel( sElement );
    if (it == m_vDropChannels.end()) 
    {
        return false;
    }
    it->enable( bEnable );
    return true;
}

bool AxDropManager::registerDropListener( IDropListener* pListener )
{
    m_vListeners.push_back( pListener );

    return true;
}


////////////////////////////////////////////////////////////
// Event Callback Methods

VARIANT_BOOL __stdcall
AxDropManager::OnHtmlElementDragEnter( IHTMLEventObj* pEvtObj )
{
    // Get the id of the element that fired the event.
    string sSourceId;
    if (!bp::ie::getSourceId( pEvtObj, sSourceId ))
    {
        BPLOG_ERROR( "getSourceId failed!" );
        return VARIANT_TRUE;
    }
    // getSourceId may return empty string for elements without an .id
    // attribute.  sDisplayName is shown in logfiles to address this.
    std::string sDisplayName = sSourceId;
    if (sDisplayName.empty()) 
    {
        sDisplayName.append("HTML element with no .id attribute");
    }

    BPLOG_DEBUG_STRM( "DragEnter from " << sDisplayName);
    
    // If the source element is not one we directly monitor, ignore this event.
    // Specifically, we want to ignore drags over children of monitored
    // drop elements.
    if (!isDropElement( sSourceId ))
    {
        BPLOG_DEBUG_STRM( "ignoring DragEnter from " << sDisplayName);
        return VARIANT_TRUE;
    }

    // Get a better interface to the event.
    // TODO: Change to use CComQIPtr.
    CComPtr<IHTMLEventObj2> htmlEvent;
    HRESULT hr = pEvtObj->QueryInterface(IID_IHTMLEventObj2,
                                         (void**)&htmlEvent);
    if (FAILED(hr))
    {
        BPLOG_COM_ERROR( "pEvtObj->QueryInterface(IHTMLEventObj2) failed!", hr );
        return VARIANT_TRUE;
    }

    // Get the data transfer object from the event.
    CComPtr<IHTMLDataTransfer> dataTransfer;
    hr = htmlEvent->get_dataTransfer(&dataTransfer);
    if (FAILED(hr))
    {
        BPLOG_COM_ERROR( "htmlEvent->get_dataTransfer() failed!", hr );
        return VARIANT_TRUE;
    }
    
    // Get a service provider from the data transfer.
    // TODO: Change to use CComQIPtr.
    CComPtr<IServiceProvider> spSrvProv;
    hr = dataTransfer->QueryInterface(IID_IServiceProvider, (void**)&spSrvProv);
    if (FAILED(hr))
    {
        BPLOG_COM_ERROR( "dataTransfer->QueryInterface(IServiceProvider) failed!",
                        hr );
        return VARIANT_TRUE;
    }
    
    // Get a data object from the event.
    CComPtr<IDataObject> pDataObject;
    const GUID SID_SDataObject = IID_IDataObject;
    hr = spSrvProv->QueryService(SID_SDataObject,
                                 IID_IDataObject,
                                 (void**)&pDataObject);
    if (FAILED(hr))
    {
        BPLOG_COM_ERROR( "QueryService for IDataObject failed!", hr );
        return VARIANT_TRUE;
    }

    // get and remember the drag items
    m_dragItems = getDragItems( pDataObject );
    tDropChannelVecIt itChan = findDropChannel( sSourceId );
    if (itChan == m_vDropChannels.end()) 
    {
        BPLOG_ERROR_STRM( "Unable to find DropChannel " << sSourceId );
        return VARIANT_TRUE;
    }
    itChan->enter( m_dragItems );
    bool bCanAccept = itChan->canAcceptDrop();
   
    // Tell all our listeners.
//  for_each( m_vListeners.begin(), m_vListeners.end(),
//            boost::bind( &IDropListener::onHover, _1, sSourceId, bCanAccept ));
    for (tListenerVecIt it=m_vListeners.begin(); it!=m_vListeners.end(); ++it)
        (*it)->onHover( sSourceId, bCanAccept );
    
    // Disable the default action, which allows OnDrop to be called.
    // see the ondrop remarks at:
    // http://msdn.microsoft.com/library/default.asp?url=/workshop/author/dhtml/reference/events/ondrop.asp?frame=true
    CComVariant vFalse = VARIANT_FALSE;
    pEvtObj->put_returnValue(vFalse);
    return VARIANT_TRUE;
}


VARIANT_BOOL __stdcall
AxDropManager::OnHtmlElementDragLeave( IHTMLEventObj* pEvtObj )
{
    // Get the id of the element that fired the event.
    string sSourceId;
    if (!bp::ie::getSourceId( pEvtObj, sSourceId ))
    {
        BPLOG_ERROR( "getSourceId failed!" );
        return VARIANT_TRUE;
    }

    BPLOG_DEBUG_STRM( "DragLeave from " << sSourceId);
    
    // If the source element is not one we directly monitor, ignore this event.
    // Specifically, we want to ignore drags over children of monitored
    // drop elements.
    if (!isDropElement( sSourceId ))
    {
        BPLOG_DEBUG_STRM( "ignoring DragLeave from " << sSourceId);
        return VARIANT_TRUE;
    }

    tDropChannelVecIt itChan = findDropChannel( sSourceId );
    if (itChan == m_vDropChannels.end()) 
    {
        BPLOG_ERROR_STRM( "Unable to find DropChannel " << sSourceId );
        return VARIANT_TRUE;
    }
    itChan->leave( false );

    // Tell all our listeners.
//  for_each( m_vListeners.begin(), m_vListeners.end(),
//            boost::bind( &IDropListener::onHover, _1, sSourceId, false ) );
    for (tListenerVecIt it=m_vListeners.begin(); it!=m_vListeners.end(); ++it)
        (*it)->onHover( sSourceId, false );

    // Disable the default action, which allows OnDrop to be called.
    // see the ondrop remarks at:
    // http://msdn.microsoft.com/library/default.asp?url=/workshop/author/dhtml/reference/events/ondrop.asp?frame=true
    CComVariant vFalse = VARIANT_FALSE;
    pEvtObj->put_returnValue(vFalse);
    return VARIANT_TRUE;
}


VARIANT_BOOL __stdcall
AxDropManager::OnHtmlElementDragOver( IHTMLEventObj* pEvtObj )
{
    // Disable the default action, which allows OnDrop to be called.
    // See the ondrop remarks at:
    // http://msdn.microsoft.com/library/default.asp?url=/workshop/author/dhtml/reference/events/ondrop.asp?frame=true
    CComVariant vFalse = VARIANT_FALSE;
    pEvtObj->put_returnValue(vFalse);
    return VARIANT_TRUE;
}


VARIANT_BOOL __stdcall
AxDropManager::OnHtmlElementDrop( IHTMLEventObj* pEvtObj )
{
    // Get the id of the element that fired the event.
    string sSourceId;
    if (!bp::ie::getSourceId( pEvtObj, sSourceId ))
    {
        BPLOG_ERROR( "getSourceId failed!" );
        return VARIANT_TRUE;
    }

    BPLOG_DEBUG_STRM( "Drop from " << sSourceId);
    
    // Get an appropriate id for the drop source.
    if (!getDropSourceId( pEvtObj, sSourceId ))
    {
        BPLOG_ERROR( "getDropSourceId failed!" );
        return VARIANT_TRUE;
    }

    BPLOG_DEBUG_STRM( "Reporting Drop from " << sSourceId);

    tDropChannelVecIt itChan = findDropChannel( sSourceId );
    if (itChan == m_vDropChannels.end()) 
    {
        BPLOG_ERROR_STRM( "Unable to find DropChannel " << sSourceId );
        return VARIANT_TRUE;
    }
    if (itChan->canAcceptDrop() == false) 
    {
        return VARIANT_TRUE;
    }
    bp::Object* dropObj = itChan->dropItems();

    for (itChan = m_vDropChannels.begin(); itChan != m_vDropChannels.end(); ++itChan) 
    {
        itChan->leave( true );
    }

    // Notify our listeners of the drop.
//  for_each( m_vListeners.begin(), m_vListeners.end(),
//            boost::bind( &IDropListener::onDrop, _1, sSourceId, dropObj ) );
    for (tListenerVecIt it=m_vListeners.begin(); it!=m_vListeners.end(); ++it)
        (*it)->onDrop( sSourceId, dropObj );

    // Tell our listeners hover off after the drop.
//  for_each( m_vListeners.begin(), m_vListeners.end(),
//            boost::bind( &IDropListener::onHover, _1, sSourceId, false ) );
    for (tListenerVecIt it=m_vListeners.begin(); it!=m_vListeners.end(); ++it)
        (*it)->onHover( sSourceId, false );

    delete dropObj;
    
    return VARIANT_TRUE;
}


void __stdcall
AxDropManager::OnWindowUnload(IHTMLEventObj* /*pEvtObj*/)
{
    stopListeningForWindowUnload();
    
    disconnectAllDropChannels();
}



//////////////////////////////////////////////////////////////
// Protected methods to be used by those that derive from us

bool
AxDropManager::handleBrowserAttach( CComPtr<IWebBrowser2>& browser )
{
    m_browser = browser;

    if (!listenForWindowUnload())
    {
        BPLOG_ERROR( "listenForWindowUnload() failed!" );
        return false;
    }

    return true;
}


bool
AxDropManager::handleBrowserDetach()
{
    if (!stopListeningForWindowUnload())
    {
        BPLOG_ERROR( "stopListeningForWindowUnload() failed!" );
        return false;
    }

    if (!disconnectAllDropChannels())
    {
        BPLOG_ERROR( "disconnectAllDropChannels() failed!" );
        return false;
    }

    m_browser = 0;

    return true;
}



////////////////////////////////////////////////////////////
// Services used by DropChannel

CComPtr<IWebBrowser2>
AxDropManager::browser()
{
    return m_browser;
}


bool
AxDropManager::connectDropChannel( int nChanNum,
                                   CComPtr<IHTMLTextContainer>& elem )
{
    if ((nChanNum < 0) || (nChanNum >= AX_NUM_DROP_CHANNELS))
    {
        BPLOG_ERROR( "Invalid channel number!" );
        return false;
    }

    HRESULT hr = connectDropChannelImpl( nChanNum, elem );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "connectDropChannelImpl failed!", hr );
        return false;
    }

    return true;
}


bool
AxDropManager::disconnectDropChannel( int nChanNum,
                                      CComPtr<IHTMLTextContainer>& elem )
{
    if ((nChanNum < 0) || (nChanNum >= AX_NUM_DROP_CHANNELS))
    {
        BPLOG_ERROR( "Invalid channel number!" );
        return false;
    }

    HRESULT hr = disconnectDropChannelImpl( nChanNum, elem );
    if (FAILED( hr ))
    {
        BPLOG_COM_ERROR( "disconnectDropChannelImpl failed!", hr );
        return false;
    }

    return true;
}



////////////////////////////////////////////////////////////
// Internal Methods

HRESULT
AxDropManager::connectDropChannelImpl( int nChanNum,
                                       CComPtr<IHTMLTextContainer>& elem )
{
    IID iid = DIID_HTMLTextContainerEvents2;
    
    switch (nChanNum)
    {
        // TODO: maybe a template function could call/get proper type/func?
        case 0:
            return HtmlTextContainerEventSink0::DispEventAdvise( elem, &iid );
        case 1:
            return HtmlTextContainerEventSink1::DispEventAdvise( elem, &iid );
        case 2:
            return HtmlTextContainerEventSink2::DispEventAdvise( elem, &iid );
        case 3:
            return HtmlTextContainerEventSink3::DispEventAdvise( elem, &iid );
        case 4:
            return HtmlTextContainerEventSink4::DispEventAdvise( elem, &iid );
        case 5:
            return HtmlTextContainerEventSink5::DispEventAdvise( elem, &iid );
        case 6:
            return HtmlTextContainerEventSink6::DispEventAdvise( elem, &iid );
        case 7:
            return HtmlTextContainerEventSink7::DispEventAdvise( elem, &iid );
        default:
            BPLOG_ERROR( "Unexpected channel number!" );
            return E_FAIL;
    }
}


HRESULT
AxDropManager::disconnectDropChannelImpl( int nChanNum,
                                          CComPtr<IHTMLTextContainer>& elem )
{
    IID iid = DIID_HTMLTextContainerEvents2;

    switch (nChanNum)
    {
        // TODO: maybe a template function could call/get proper type/func?
        case 0:
            return HtmlTextContainerEventSink0::DispEventUnadvise( elem, &iid );
        case 1:
            return HtmlTextContainerEventSink1::DispEventUnadvise( elem, &iid );
        case 2:
            return HtmlTextContainerEventSink2::DispEventUnadvise( elem, &iid );
        case 3:
            return HtmlTextContainerEventSink3::DispEventUnadvise( elem, &iid );
        case 4:
            return HtmlTextContainerEventSink4::DispEventUnadvise( elem, &iid );
        case 5:
            return HtmlTextContainerEventSink5::DispEventUnadvise( elem, &iid );
        case 6:
            return HtmlTextContainerEventSink6::DispEventUnadvise( elem, &iid );
        case 7:
            return HtmlTextContainerEventSink7::DispEventUnadvise( elem, &iid );
        default:
            BPLOG_ERROR( "Unexpected channel number!" );
            return E_FAIL;
    }
}


bool
AxDropManager::disconnectAllDropChannels()
{
    for_each( m_vDropChannels.begin(), m_vDropChannels.end(),
              mem_fun_ref( &DropChannel::disconnect ) );
    
    return true;
}


// Returns the drop channel directly monitoring the specified
// element, or m_vDropChannels.end() if elem not monitored.
AxDropManager::tDropChannelVecIt
AxDropManager::findDropChannel( const std::string& sElemId )
{
    // no valid drop channel may ever have an empty id
    if (sElemId.empty()) return m_vDropChannels.end();
	
//  return find_if( m_vDropChannels.begin(), m_vDropChannels.end(),
//                  boost::bind( &DropChannel::sourceId, _1 ) == sElemId );
    tDropChannelVecIt it;
    for (it = m_vDropChannels.begin(); it != m_vDropChannels.end(); ++it)
        if (it->sourceId() == sElemId)
            break;
    return it;
}


std::vector<bp::file::Path> 
AxDropManager::getDragItems( IDataObject* pDataObject )
{
    std::vector<bp::file::Path> rval;

    FORMATETC fmt;
    fmt.ptd      = 0;
    fmt.dwAspect = DVASPECT_CONTENT;
    fmt.lindex   = -1;
    fmt.tymed    = TYMED_HGLOBAL;
    fmt.cfFormat = CF_HDROP;

    // Determine if we can get files.
    HRESULT hr = pDataObject->QueryGetData(&fmt);
    if (FAILED(hr))
    {
        BPLOG_COM_ERROR("pDataObject->QueryGetData() failed", hr);
        return rval;
    }

    STGMEDIUM medium;
    medium.tymed          = TYMED_HGLOBAL;
    medium.hGlobal        = 0;
    medium.pUnkForRelease = 0;
    // Get the data from the drop as an HDROP.
    hr = pDataObject->GetData(&fmt, &medium);
    if (FAILED(hr))
    {
        BPLOG_COM_ERROR("pDataObject->GetData() failed", hr);
        return rval;
    }

    HDROP hdrop = (HDROP)medium.hGlobal;

    // Get the total number of files dropped.
    UINT uNumFiles = DragQueryFile(hdrop, static_cast<UINT>(-1), NULL, 0);
    for (UINT i = 0; i < uNumFiles; ++i)
    {
        // Get this file.
        wchar_t wsFilename[MAX_PATH];
        if (DragQueryFileW(hdrop, i, wsFilename, MAX_PATH) > 0)
        {
            rval.push_back(bp::file::Path(wsFilename));
        }
    }
    
    // Done w/ HDROP.
    ReleaseStgMedium(&medium);

    return rval;
}


// Gets an appropriate source id from the provided event.
bool
AxDropManager::getDropSourceId( IHTMLEventObj* pEvtObj,
                                 std::string& sSourceId )
{
    // Current behavior: just return source element id.
//  return bp::ie::getSourceId( pEvtObj, sSourceId );

    // 2008feb26 dg (YIB-1769913)
    // When an html child element without a drop handler is attached to
    // an element with a handler, what happens is that our handlers get
    // called but they receive the source id of the child element,
    // which is then unrecognized in the pluglet.
    // To deal with this:
    //   1) Check if the event source id is being monitored by one of our drop
    //   channels.  If so then this is not a child element and we may
    //   pass this id to the pluglet as normal.
    //   2) If test 1 fails then see if the event source is a child of any
    //   element being monitored by one of our drop channels.  If so
    //   then pass the parent id to the pluglet.
    //   3) If test 2 fails then just pass the raw source id.  In this
    //   case we'll capture the event but not doing anything with it.

    // Get the reported source id.
    string sReportedId;
    if (!bp::ie::getSourceId( pEvtObj, sReportedId ))
    {
        BPLOG_ERROR( "getSourceId failed!" );
        return false;
    }
    
    // If we have a channel explicitly connected to the specified
    // source element, use that as the source id.
    if (isDropElement( sReportedId ))
    {
        sSourceId = sReportedId;
        return true;
    }

    // See if any of our drop channels are monitoring a parent of this
    // element.
    for( tDropChannelVecIt itChan = m_vDropChannels.begin();
         itChan != m_vDropChannels.end(); itChan++)
    {
        std::string sChanElemId = itChan->sourceId();
        
        // Get the html element the channel is monitoring.
        // TODO: DropChannel's could potentially hold this as a member
        CComPtr<IHTMLElement> elemChan;
        if (!bp::ie::getDocElementById( m_browser, sChanElemId, elemChan ))
        {
            BPLOG_ERROR( "getDocElementById failed!" );
            return false;
        }

        // Get the reported html source element.
        CComPtr<IHTMLElement> elemReported;
        HRESULT hr = pEvtObj->get_srcElement( &elemReported );
        if (FAILED( hr ))
        {
            BPLOG_COM_ERROR( "pEvtObj->get_srcElement failed!", hr );
            return false;
        }
        
        // If the reported element is a child of the channel element,
        // use channel element id.
        VARIANT_BOOL bContains;
        hr = elemChan->contains( elemReported, &bContains );
        if (FAILED( hr ))
        {
            BPLOG_COM_ERROR( "elemChan->contains failed!", hr );
            return false;
        }

        if (bContains == VARIANT_TRUE)
        {
            sSourceId = sChanElemId;
            return true;
        }
    }

    // Bummer.  Pass on the reported id, which will be rejected in the pluglet.
    sSourceId = sReportedId;
    return true;
}


// Is the specified dom element id connected to (monitored by) one
// of our drop channels.
bool
AxDropManager::isDropElement( const std::string& sElemId )
{
    tDropChannelVecIt it = findDropChannel( sElemId );
    return((it != m_vDropChannels.end()) && it->isEnabled());
}


bool
AxDropManager::listenForWindowUnload()
{
    if (!bp::ie::getDocWindow( m_browser, m_dropWindow ))
    {
        BPLOG_ERROR( "getDocWindow() failed!" );
        return false;
    }
        
    // Start listening.
    HRESULT hr;
    hr = HtmlWindowEventSink::DispEventAdvise( m_dropWindow,
                                               &DIID_HTMLWindowEvents2 );
    if (FAILED(hr))
    {
        m_dropWindow = 0;
        BPLOG_COM_ERROR("DispEventAdvise(m_dropWindow) failed!", hr);
        return false;
    }

    return true;
}


bool 
AxDropManager::stopListeningForWindowUnload()
{
    if (!m_dropWindow)
    {
        return true;
    }
    
    // Stop listening.
    HRESULT hr;
    hr = HtmlWindowEventSink::DispEventUnadvise( m_dropWindow,
                                                 &DIID_HTMLWindowEvents2 );
    if (FAILED(hr))
    {        
        BPLOG_COM_ERROR("DispEventUnadvise(m_dropWindow) failed!", hr);
        return false;
    }
    
    m_dropWindow = 0;

    return true;
}


