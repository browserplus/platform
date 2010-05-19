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

#include "bpdefutil.h"

#include <sstream>

bp::Map *
bp::defutil::defToJson(const BPCoreletDefinition * def)
{
    if (def == NULL) return NULL;

    bp::Map * m = new bp::Map;
    
    m->add("name", new bp::String(def->coreletName));
    bp::Map * verMap = new bp::Map;
    verMap->add("major", new bp::Integer(def->majorVersion));
    verMap->add("minor", new bp::Integer(def->minorVersion));
    verMap->add("micro", new bp::Integer(def->microVersion));
    m->add("version", verMap);
    
    // now add the version string
    {
        std::stringstream ss;
        ss << def->majorVersion << '.'
           << def->minorVersion << '.'
           << def->microVersion;
        m->add("versionString", new bp::String(ss.str()));        
    }
    
    m->add("documentation", new bp::String(def->docString));

    // add functions
    unsigned int i;
    
    bp::List * funcList = new bp::List;
    for (i=0; i < def->numFunctions; i++) {
        BPFunctionDefinition * f = def->functions + i;
        if (f == NULL) continue;

        bp::Map * fm = new bp::Map;

        fm->add("name", new bp::String(f->functionName));
        fm->add("documentation", new bp::String(f->docString));        

        // add arguments
        unsigned int j;
        
        bp::List * paramList = new bp::List;
        for (j=0; j < f->numArguments; j++) {
            BPArgumentDefinition * a = f->arguments + j;            
            bp::Map * am = new bp::Map;

            am->add("name", new bp::String(a->name));
            am->add("documentation", new bp::String(a->docString));
            am->add("required", new bp::Bool(a->required));
            am->add("type", new bp::String(bp::typeAsString(a->type)));
            
            paramList->append(am);
        }
        fm->add("parameters", paramList);
        funcList->append(fm);
    }
    m->add("functions", funcList);
    
    return m;
}

 
bp::List *
bp::defutil::defsToJson(const BPCoreletDefinition ** defs,
                        unsigned int numDefs)
{
    if (defs == NULL || numDefs == 0) return NULL;
    
    bp::List * l = new bp::List;
    for (unsigned int i = 0; i < numDefs; ++i) {
        bp::Map* m = bp::defutil::defToJson(defs[i]);
        if (m) {
            l->append(m);
        }
    }
    return l;
}
