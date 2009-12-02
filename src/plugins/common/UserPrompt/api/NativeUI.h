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

#ifndef __NATIVEUI_H__
#define __NATIVEUI_H__

#include <string>
#include <list>

namespace bp {
    class Object;
}

namespace bp {
    namespace ui {
        /**
         * "run" a *modal* HTML dialog
         * 
         * \param parentWindow - An OS Specific argument.  A pointer
         *        to the window to which a created window much be a
         *        child.
         * \param pathToHTMLDialog - where the html to be rendered
         *        lives
         * \param userAgent - The useragent string of the host browser
         *        that we're being rendered inside.  Used to deal with
         *        browser specific behavior that affects how the
         *        window should be created.
         * \param arguments - Arguments to pass to the HTML dialog
         *        (made available via a javascript function)
         * \param oResponse - The return value of the dialog (whatever
         *        the in-prompt javascript passes to
         *        BPDialog.complete()
         *
         * \returns success indicator.  On true oResponse will contain
         *         a dynamically allocated bp::object client owns
         *         memory and must delete.
         */
        bool HTMLPrompt(void * parentWindow,
                        const std::string & pathToHTMLDialog,
                        const std::string & userAgent,
                        const bp::Object * arguments,
                        bp::Object ** oResponse);
    };
};

#endif
