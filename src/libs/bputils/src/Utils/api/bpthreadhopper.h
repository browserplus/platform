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

/************************************************************************
 * ThreadHopper - A utility to grab the attention of the thread that
 *        it's instantiated on.  In order to function the thread must
 *        be using operating system provided runloops/event pumps.
 *
 * Written by Lloyd Hilaiel, Fri Jul  6 13:58:30 MDT 2007
 * (c) 2007, Yahoo! Inc, all rights reserved.
 */

#ifndef __BPTHREADHOPPPER_H__
#define __BPTHREADHOPPPER_H__

#include <vector>

namespace bp { namespace thread {

class Hopper 
{
  public:
    Hopper();
    ~Hopper();    

    /*
     * Initialize the ThreadHopper.  The thread upon which this function
     * is invoked is the thread that function pointers passed into
     * invokeOnThread() will be invoked on.
     */
    bool initializeOnCurrentThread();

    typedef void (*InvokeFuncPtr)(void * context);

    /*
     * invoke a function on the thread upon which intializeOnCurrentThread
     * was called.
     */
    bool invokeOnThread(InvokeFuncPtr invokeFunc, void * context);

    /**
     * provide the thread of control upon which ThreadHopper was instantiated
     * to process any outstanding requests.
     */
    void processOutstandingRequests();

  private:
    // internal os specific stuff
    void * m_osSpecific;
};

/**
 * A class which one may derive from to make hopping easier and
 * automatically pass the 'this' pointer.
 *
 * A threadhopper will be contained, shut down at destruction, and
 * initialized on the thread of control that constructs the derived
 * class.
 *
 * This class is focues on the use case of wanting to hop through
 * the message pump to invoke a callback after function return.  By 
 * making the gaurantee that all callbacks will be invoked after
 * function completion, clients of derived classes will have more
 * predictable usage semantics and hopefully have to write less code.
 *
 * Finally, the alternate implementation tactics would be to design
 * functions which return synchronously or asynchronously, or invoke
 * client callbacks before invoked functions return.  The former
 * requires clients to create additional functions to capture
 * commonality and the latter is often unintuitive.
 */
class HoppingClass
{
  public:
    HoppingClass();
    virtual ~HoppingClass();

  protected:
    void hop(void * context);
    virtual void onHop(void * context) = 0;

  private:
    Hopper m_hopper;
    static void hoppingClassRelayFunc(void *);
};


} }

#endif
