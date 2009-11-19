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
 *  ServicesUpdatedFile.h
 *
 *  A simple abstraction around a temporal disk file that may be
 *  written (touched) by processes which install or modify services on
 *  disk to convey to a running BrowserPlusCore daemon that it's time
 *  to rescan services.  This allows a very low runtime cost for near
 *  instant detection of newly installed services
 *
 *  Created by Lloyd Hilaiel on 01/27/09.
 *  Copyright 2009 Yahoo! Inc. All rights reserved.
 *
 */

#ifndef __SERVICESUPDATED__ 
#define __SERVICESUPDATED__ 

namespace ServicesUpdated 
{
    /**
     * create a small file in the user scoped "Corelets" dir signifying
     * that services on disk have been changed.
     */
    void indicateServicesChanged();

    /** check if services on disk have been changed, if so, delete the
     *  file and return true.  false otherwise */
    bool servicesChanged();
};

#endif
