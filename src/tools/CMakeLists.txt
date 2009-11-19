# ***** BEGIN LICENSE BLOCK *****
# The contents of this file are subject to the Mozilla Public License
# Version 1.1 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
# 
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
# 
# The Original Code is BrowserPlus (tm).
# 
# The Initial Developer of the Original Code is Yahoo!.
# Portions created by Yahoo! are Copyright (C) 2006-2009 Yahoo!.
# All Rights Reserved.
# 
# Contributor(s): 
# ***** END LICENSE BLOCK *****
ADD_SUBDIRECTORY( bpargvtest )
ADD_SUBDIRECTORY( bpclient )
ADD_SUBDIRECTORY( bpkg )
ADD_SUBDIRECTORY( bplocale )
ADD_SUBDIRECTORY( bpproto_stress )
ADD_SUBDIRECTORY( bptar )
ADD_SUBDIRECTORY( bpwebserve )
ADD_SUBDIRECTORY( bpwget )
ADD_SUBDIRECTORY( cli_prog_sample )
ADD_SUBDIRECTORY( distro_client )
ADD_SUBDIRECTORY( ipcclient )
ADD_SUBDIRECTORY( ipcserver )
ADD_SUBDIRECTORY( service_manager )

IF ( WIN32 )
  ADD_SUBDIRECTORY( install_bootstrapper )
ENDIF ( WIN32 )
