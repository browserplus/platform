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
 * HTMLWindow.h - an abstraction around running a window which displays
 *                HTML and exposes scriptable functions into the
 *                running javascript context. 
 */

#include "api/HTMLWindow.h"


bp::html::HTMLWindow::HTMLWindow()
{
}

bp::html::HTMLWindow::~HTMLWindow()
{
}


void
bp::html::HTMLWindow::setDimensions(int , int )
{
}

void
bp::html::HTMLWindow::setParent(void * )
{
}

bool
bp::html::HTMLWindow::render(const std::string &)
{
	return false;
}

bool
bp::html::HTMLWindow::show()
{
	return false;
}

bool
bp::html::HTMLWindow::hide()
{
	return false;
}
