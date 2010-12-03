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
 *  AxDropManager.h
 *
 *  ActiveX Drag and Drop manager.
 *  
 *  Created by David Grigsby on 11/09/2007.
 *  Copyright 2007 Yahoo! Inc. All rights reserved.
 *
 */
#ifndef __AXDROPMANAGER_H__
#define __AXDROPMANAGER_H__

#include "BPUtils/bpfile.h"
#include "PluginCommonLib/IDropManager.h"
#include "DropChannel.h"


// Forward Decls
class IDropListener;


// We have to setup connection points at compile time for
// each independent drop target.
// This macro establishes the number of channels.
// Some code in this file must be maintained manually to correspond
// with this value.
#define AX_NUM_DROP_CHANNELS    8


// We need a distinct C++ type for each ATL connection point.  Bummer.
// TODO: Boost looping macro?

// This macro declares types of the form HtmlTextContainerEventSink0, 1, etc.
#define AX_DECLARE_CONTAINER_EVENTSINK_TYPE(n) \
typedef IDispEventImpl<n, class AxDropManager, \
                       &DIID_HTMLTextContainerEvents2, \
                       &LIBID_MSHTML,4,0> HtmlTextContainerEventSink##n

AX_DECLARE_CONTAINER_EVENTSINK_TYPE(0);
AX_DECLARE_CONTAINER_EVENTSINK_TYPE(1);
AX_DECLARE_CONTAINER_EVENTSINK_TYPE(2);
AX_DECLARE_CONTAINER_EVENTSINK_TYPE(3);
AX_DECLARE_CONTAINER_EVENTSINK_TYPE(4);
AX_DECLARE_CONTAINER_EVENTSINK_TYPE(5);
AX_DECLARE_CONTAINER_EVENTSINK_TYPE(6);
AX_DECLARE_CONTAINER_EVENTSINK_TYPE(7);


#define AX_WINDOW_EVENT_ID  100
typedef IDispEventImpl<AX_WINDOW_EVENT_ID, class AxDropManager,
                       &DIID_HTMLWindowEvents2,
                       &LIBID_MSHTML,4,0> HtmlWindowEventSink;

// This macro declares desired sink entries for a particular text container.
#define AX_DECLARE_CONTAINER_SINK_ENTRIES(n) \
SINK_ENTRY_EX(n, \
              DIID_HTMLTextContainerEvents2, \
              DISPID_HTMLELEMENTEVENTS2_ONDROP, \
              OnHtmlElementDrop) \
SINK_ENTRY_EX(n, \
              DIID_HTMLTextContainerEvents2, \
              DISPID_HTMLELEMENTEVENTS2_ONDRAGOVER, \
              OnHtmlElementDragOver) \
SINK_ENTRY_EX(n, \
              DIID_HTMLTextContainerEvents2, \
              DISPID_HTMLELEMENTEVENTS2_ONDRAGENTER, \
              OnHtmlElementDragEnter) \
SINK_ENTRY_EX(n, \
              DIID_HTMLTextContainerEvents2, \
              DISPID_HTMLELEMENTEVENTS2_ONDRAGLEAVE, \
              OnHtmlElementDragLeave)



// TODO: Can this be used as a member rather than a base class?
class AxDropManager :
    public IDropManager,
    public HtmlTextContainerEventSink0,
    public HtmlTextContainerEventSink1,
    public HtmlTextContainerEventSink2,
    public HtmlTextContainerEventSink3,
    public HtmlTextContainerEventSink4,
    public HtmlTextContainerEventSink5,
    public HtmlTextContainerEventSink6,
    public HtmlTextContainerEventSink7,
    public HtmlWindowEventSink
{
public:
    AxDropManager();
    ~AxDropManager();

    
    ///////////////////////////
    // IDropManager Methods
    virtual bool addTarget( const std::string& sElement,
                            const std::set<std::string>& mimeTypes,
                            bool includeGestureInfo,
                            unsigned int limit );
    virtual bool addTarget( const std::string& sElement,
                            const std::string& sVersion );
    virtual bool removeTarget( const std::string& sElement );
    virtual bool enableTarget( const std::string& sElement,
                               bool bEnable );
    virtual bool registerDropListener( IDropListener* pListener );

    
    ///////////////////
    // ATL Sink Map
    BEGIN_SINK_MAP(AxDropManager)
        // TODO: Boost looping macro?
        AX_DECLARE_CONTAINER_SINK_ENTRIES(0)
        AX_DECLARE_CONTAINER_SINK_ENTRIES(1)
        AX_DECLARE_CONTAINER_SINK_ENTRIES(2)
        AX_DECLARE_CONTAINER_SINK_ENTRIES(3)
        AX_DECLARE_CONTAINER_SINK_ENTRIES(4)
        AX_DECLARE_CONTAINER_SINK_ENTRIES(5)
        AX_DECLARE_CONTAINER_SINK_ENTRIES(6)
        AX_DECLARE_CONTAINER_SINK_ENTRIES(7)

        SINK_ENTRY_EX(AX_WINDOW_EVENT_ID,
                      DIID_HTMLWindowEvents2,
                      DISPID_HTMLWINDOWEVENTS2_ONUNLOAD,
                      OnWindowUnload)
    END_SINK_MAP()


protected:
    bool handleBrowserAttach( CComPtr<IWebBrowser2>& browser );
    bool handleBrowserDetach();
    
    
private:
    //////////////////////
    // Event Callbacks
    // When someone drag-enters over the DropTarget.
    VARIANT_BOOL __stdcall OnHtmlElementDragEnter(IHTMLEventObj* pEvtObj);

    // When someone drag-leaves over the DropTarget.
    VARIANT_BOOL __stdcall OnHtmlElementDragLeave(IHTMLEventObj* pEvtObj);
    
    // When someone drags an object over the DropTarget.
    VARIANT_BOOL __stdcall OnHtmlElementDragOver(IHTMLEventObj* pEvtObj);

    // Handle drops on an HTML element (the DropTarget).
    VARIANT_BOOL __stdcall OnHtmlElementDrop(IHTMLEventObj* pEvtObj);

    // When the window unloads, calls StopListeningForDrops().
    void __stdcall OnWindowUnload(IHTMLEventObj* pEvtObj);

    
    ///////////////////////////////////
    // Services used by DropChannel
    friend class DropChannel;
    CComPtr<IWebBrowser2> browser();
    bool connectDropChannel( int nChanNum,
                             CComPtr<IHTMLTextContainer>& elem );

    bool disconnectDropChannel( int nChanNum,
                                CComPtr<IHTMLTextContainer>& elem );

    
    ///////////////////////
    // Internal Types
    typedef std::vector<DropChannel> tDropChannelVec;
    typedef tDropChannelVec::iterator tDropChannelVecIt;
    typedef std::vector<IDropListener*> tListenerVec;
    typedef tListenerVec::iterator tListenerVecIt;

    
    ///////////////////////
    // Internal Methods
    HRESULT connectDropChannelImpl( int nChanNum,
                                    CComPtr<IHTMLTextContainer>& elem );

    HRESULT disconnectDropChannelImpl( int nChanNum,
                                       CComPtr<IHTMLTextContainer>& elem );

    bool disconnectAllDropChannels();

    // Returns the drop channel directly monitoring the specified
    // element, or m_vDropChennels.end() if elem not monitored.
    tDropChannelVecIt findDropChannel( const std::string& sElemId );
    
    std::vector<boost::filesystem::path> getDragItems( IDataObject* pDataObject );

    // Get an appropriate source id for a drop event.
    // If the drop was on a monitored element or one of its children,
    // use the id of the monitored element.
    bool getDropSourceId( IHTMLEventObj* pEvtObj, std::string& sSourceId );

    // Is the specified dom element id connected to (monitored by) one
    // of our drop channels.
    bool isDropElement( const std::string& sElemId );
    
    bool listenForWindowUnload();
    
    bool stopListeningForWindowUnload();
    
    
    /////////////////
    // Attributes
    CComPtr<IWebBrowser2>   m_browser;
    CComPtr<IHTMLWindow2>   m_dropWindow;
    tDropChannelVec         m_vDropChannels;
    tListenerVec            m_vListeners;

    std::vector<boost::filesystem::path> m_dragItems;

    //////////////////////
    // Prevent copying
    AxDropManager( const AxDropManager& );
    AxDropManager& operator=( const AxDropManager& );
};


#endif // __AXDROPMANAGER_H__
