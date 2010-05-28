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

/**
 * bputil.hh -- c++ utilities to make building hierarchies of BPElements
 *              eaiser.  A tool that may be consumed in source form
 *              by a service author to simplify mapping into and out of
 *              introspectable service API types.
 */

#include "bptypeutil.h"
#include "bperrorutil.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include "bpstrutil.h"

#ifdef WIN32
#pragma warning(disable:4100)
#endif

const char *
bp::typeAsString(BPType t)
{
    switch (t) {
        case BPTNull: return "null";
        case BPTBoolean: return "boolean";
        case BPTInteger: return "integer";
        case BPTDouble: return "double";
        case BPTString: return "string";
        case BPTMap: return "map";
        case BPTList: return "list";
        case BPTCallBack: return "callback";
        case BPTNativePath: return "path";
        case BPTAny: return "any";
    }
    return "unknown";
}


bp::Object::Object(BPType t)
{
    e.type = t;
}

bp::Object::~Object()
{
}

BPType bp::Object::type() const
{
    return e.type;
}

bool
bp::Object::has(const char * path, BPType type) const
{
    const bp::Object * obj = get(path);
    return ((obj != NULL) && obj->type() == type);
}

bool
bp::Object::has(const char * path) const
{
    return (get(path) != NULL);
}

const bp::Object *
bp::Object::get(const char * path) const
{
    const Object * obj = NULL;

    if (path == NULL) return obj;
    
    std::vector<std::string> paths =
        bp::strutil::split(std::string(path), "/");

    obj = this;

    for (unsigned int i = 0; i < paths.size(); i++) {
        if (obj->type() != BPTMap) {
            obj = NULL;
            break;
        }
        const Object * oldobj = obj;        
        obj = NULL;
        for (unsigned int j = 0; j < oldobj->e.value.mapVal.size; j++)
        {
            if (!paths[i].compare(oldobj->e.value.mapVal.elements[j].key))
            {
                obj = static_cast<const bp::Map *>(oldobj)->values[j];
                break;
            }
        }
        if (obj == NULL) break;
    }
    
    return obj;
}

const BPElement *
bp::Object::elemPtr() const
{
    return &e;
}

bp::Object *
bp::Object::build(const BPElement * elem)
{
    Object * obj = NULL;

    if (elem != NULL)
    {
        switch (elem->type)
        {
            case BPTNull:
                obj = new bp::Null;
                break;
            case BPTBoolean:
                obj = new bp::Bool(elem->value.booleanVal);
                break;
            case BPTInteger:
                obj = new bp::Integer(elem->value.integerVal);
                break;
            case BPTCallBack:
                obj = new bp::CallBack(elem->value.callbackVal);
                break;
            case BPTDouble:
                obj = new bp::Double(elem->value.doubleVal);
                break;
            case BPTString:
                obj = new bp::String(elem->value.stringVal);
                break;
            case BPTNativePath: 
            {
                obj = new bp::Path(bp::file::Path(elem->value.pathVal));
                break;
            }
            case BPTMap:
            {
                bp::Map * m = new bp::Map;
                
                for (unsigned int i = 0; i < elem->value.mapVal.size; i++)
                {
                    m->add(elem->value.mapVal.elements[i].key,
                           build(elem->value.mapVal.elements[i].value));
                }

                obj = m;
                break;
            }
            case BPTList:
            {
                bp::List * l = new bp::List;
                
                for (unsigned int i = 0; i < elem->value.listVal.size; i++)
                {
                    l->append(build(elem->value.listVal.elements[i]));
                }

                obj = l;
                break;
            }
            case BPTAny: {
                // invalid
                break;
            }
        }
    }
    
    return obj;
}

const char *
bp::Object::getStringNodeValue( const char * cszPath )
{
    const Object* pNode = get(cszPath);
    if (!pNode || pNode->type() != BPTString)
    {
        return NULL;
    }
    
    return static_cast<const bp::String *>(pNode)->value();
}

bp::Object::operator bool() const 
{
    throw ConversionException("cannot convert to bool");
}

bp::Object::operator std::string() const 
{
    throw ConversionException("cannot convert to string");
}

bp::Object::operator bp::file::Path() const 
{
    throw ConversionException("cannot convert to path");
}

bp::Object::operator long long() const 
{
    throw ConversionException("cannot convert to long");
}

bp::Object::operator double() const 
{
    throw ConversionException("cannot convert to double");
}

bp::Object::operator std::map<std::string, const bp::Object *>() const
{
    throw ConversionException("cannot convert to map<string, Object*>");
}
    

bp::Object::operator std::vector<const bp::Object *>() const
{
    throw ConversionException("cannot convert to vector<Object*>");
}

const bp::Object &
bp::Object::operator[](const char *) const
{
    throw ConversionException("cannot apply operator[const char*]");
}

const bp::Object &
bp::Object::operator[](unsigned int) const
{
    throw ConversionException("cannot apply operator[int]");
}

bp::Null::Null()
    : bp::Object(BPTNull) 
{
}

bp::Null::~Null()
{
}

bp::Object * 
bp::Null::clone() const
{
    return new bp::Null();
}
    
bp::Bool::Bool(bool val)
    : bp::Object(BPTBoolean) 
{
    e.value.booleanVal = val;
}

bp::Bool::~Bool()
{
}

BPBool
bp::Bool::value() const
{
    return e.value.booleanVal;
}

bp::Object * 
bp::Bool::clone() const
{
    return new bp::Bool(value());
}

bp::Bool::operator bool() const 
{
    return value();
}

bp::String::String(const char * str)
    : bp::Object(BPTString) 
{
    if (!str) str = "";
    this->str.append(str);
    e.value.stringVal = (char *) this->str.c_str();
}

bp::String::String(const char * str, unsigned int len)
    : bp::Object(BPTString) 
{
    this->str.append(str, len);
    e.value.stringVal = (char *) this->str.c_str();
}

bp::String::String(const std::string & str)
    : bp::Object(BPTString) 
{
    this->str = str;
    e.value.stringVal = (char *) this->str.c_str();
}

bp::String::String(const String & other)
    : bp::Object(BPTString)
{
    str = other.str;
    e.value.stringVal = (char *) this->str.c_str();
}

bp::String &
bp::String::operator= (const String & other)
{
    str = other.str;
    e.value.stringVal = (char *) this->str.c_str();
    return *this;
}

bp::String::~String()
{
}

const BPString
bp::String::value() const
{
    return e.value.stringVal;
}

bp::Object * 
bp::String::clone() const
{
    return new String(*this);
}

bp::String::operator std::string() const 
{
    return std::string(value());
}

bp::Path::Path(const bp::file::Path & path)
    : bp::Object(BPTNativePath), m_path(path.external_file_string())
{
    e.value.pathVal = (BPPath) m_path.c_str();
}

bp::Path::Path(const Path & other)
    :  bp::Object(BPTNativePath), m_path(other.m_path)
{
    e.type = BPTNativePath;
    e.value.pathVal = (BPPath) m_path.c_str();
}

const BPPath
bp::Path::value() const
{
    return e.value.pathVal;
}

bp::Path &
bp::Path::operator= (const Path & other)
{
    m_path = other.m_path;
    e.value.pathVal = (BPPath) m_path.c_str();
    return *this;
}

bp::Path::operator bp::file::Path() const 
{
	return bp::file::Path(m_path);
}

bp::Path::~Path()
{
}

bp::Object * 
bp::Path::clone() const
{
// 2009apr29 dg
//  return new Path(this->str);
    return new Path(*this);
}

bp::Map::Map() : bp::Object(BPTMap) 
{
    e.value.mapVal.size = 0;
    e.value.mapVal.elements = NULL;
}


bp::Map::Map(const Map & o) : bp::Object(BPTMap) 
{
    e.value.mapVal.size = 0;
    e.value.mapVal.elements = NULL;

    Iterator i(o);
    const char * k;
    while (NULL != (k = i.nextKey())) {
        add(k, o.value(k)->clone());
    }
}

bp::Map &
bp::Map::operator= (const bp::Map & o)
{
    for (unsigned int i = 0; i < values.size(); i++) delete values[i];
    if (e.value.mapVal.elements != NULL) free(e.value.mapVal.elements);
    memset((void *) &e, 0, sizeof(e));
    values.clear(); keys.clear();

    e.type = BPTMap;    
    Iterator i(o);
    const char * k;
    while (NULL != (k = i.nextKey())) {
        add(k, o.value(k)->clone());
    }

    return *this;
}

bp::Object *
bp::Map::clone() const
{
    return new Map(*this);
}

bp::Map::~Map()
{
    for (unsigned int i = 0; i < values.size(); i++)
    {
        delete values[i];
    }
    
    if (e.value.mapVal.elements != NULL) 
    {
        free(e.value.mapVal.elements);
    }
}


unsigned int
bp::Map::size() const
{
    return e.value.mapVal.size;
}


const bp::Object *
bp::Map::value(const char * key) const
{
	if (key == NULL) return NULL;
    unsigned int i;
    std::vector<std::string>::const_iterator it;
    for (i = 0, it = keys.begin(); it != keys.end(); it++, i++) {
        if (!strcmp(key, (*it).c_str()))
            return values[i];
    }
    return NULL;
}

const bp::Object &
bp::Map::operator[](const char * key) const
{
    const bp::Object * v = value(key);
    if (v == NULL) {
        BP_THROW_TYPE(ConversionException,"no such element in map");
    }
    return *v;
}

bool
bp::Map::kill(const char * key)
{
    bool rval = false;
	if (key == NULL) return rval;
    std::vector<std::string>::iterator it;
    std::vector<bp::Object*>::iterator vit;
    for (it = keys.begin(), vit = values.begin();
         it != keys.end() && vit != values.end();
         ++it, ++vit) {
        if (!strcmp(key, (*it).c_str())) {
            keys.erase(it);
            delete *vit;
            values.erase(vit);
            e.value.mapVal.size--;
            rval = true;
            break;
        }
    }
    
    // if we found and removed key, rebuild BPElements
    if (rval) {
        e.value.mapVal.elements =
            (BPMapElem *) realloc(e.value.mapVal.elements,
                                  sizeof(BPMapElem) * e.value.mapVal.size);
        for (unsigned int ix = 0; ix < e.value.mapVal.size; ix++)
        {
            e.value.mapVal.elements[ix].key = (BPString) keys[ix].c_str();
            e.value.mapVal.elements[ix].value = (BPElement *) values[ix]->elemPtr();

        }
    }
    return rval;
}

void
bp::Map::add(const char * key, bp::Object * value)
{
    BPASSERT(value != NULL);
    kill(key);
    unsigned int ix = e.value.mapVal.size;
    e.value.mapVal.size++;
    values.push_back(value);
    e.value.mapVal.elements =
        (BPMapElem *) realloc(e.value.mapVal.elements,
                              sizeof(BPMapElem) * e.value.mapVal.size);
    e.value.mapVal.elements[ix].value = (BPElement *) value->elemPtr();    
    // adding a key may cause some strings to be reallocated (!).  after
    // the addition we must update key ptrs
    keys.push_back(key);
	for (ix = 0; ix < e.value.mapVal.size; ix++)
	{
		e.value.mapVal.elements[ix].key = (BPString) keys[ix].c_str();
	}
}

void
bp::Map::add(const std::string& key, bp::Object* value)
{
    add(key.c_str(), value);
}

bool
bp::Map::getBool(const std::string& sPath, bool& bValue) const
{
    if (has(sPath.c_str(), BPTBoolean)) {
        bValue = dynamic_cast<const bp::Bool*>(get(sPath.c_str()))->value();
        return true;
    }

    return false;
}

bool
bp::Map::getInteger(const std::string& sPath, int& nValue) const
{
    if (has(sPath.c_str(), BPTInteger)) {
        long long int lVal = dynamic_cast<const Integer*>(get(sPath.c_str()))->value();
        nValue = static_cast<int>(lVal);
        return true;
    }

    return false;
}
   
bool
bp::Map::getList(const std::string& sPath, const bp::List*& pList) const
{
    if (has(sPath.c_str(), BPTList)) {
        pList = dynamic_cast<const bp::List*>(get(sPath.c_str()));
        return true;
    }

    return false;
}

bool
bp::Map::getLong(const std::string& sPath, long long int& lValue) const
{
    if (has(sPath.c_str(), BPTInteger)) {
        lValue = dynamic_cast<const bp::Integer*>(get(sPath.c_str()))->value();
        return true;
    }

    return false;
}

bool
bp::Map::getMap(const std::string& sPath, const bp::Map*& pMap) const
{
    if (has(sPath.c_str(), BPTMap)) {
        pMap = dynamic_cast<const bp::Map*>(get(sPath.c_str()));
        return true;
    }

    return false;
}

bool
bp::Map::getString(const std::string& sPath, std::string& sValue) const
{
    if (has(sPath.c_str(), BPTString)) {
        sValue = dynamic_cast<const bp::String*>(get(sPath.c_str()))->value();
        return true;
    }

    return false;
}

bp::Map::Iterator::Iterator(const class bp::Map& m) {
    m_it = m.keys.begin();
    m_m = &m;
}

const char *
bp::Map::Iterator::nextKey()
{
    if (m_it == m_m->keys.end()) return NULL;
    const char * key = (*m_it).c_str();
    m_it++;
    return key;
}

bp::Map::operator std::map<std::string, const bp::Object *>() const
{
    std::map<std::string, const bp::Object *> m;
    Iterator i(*this);
    const char * k;
    while (NULL != (k = i.nextKey())) m[k] = value(k);
    return m;
    
}

bp::Integer::Integer(BPInteger num)
    : Object(BPTInteger)
{
    e.value.integerVal = num;
}

bp::Integer::~Integer()
{
}

BPInteger
bp::Integer::value() const
{
    return e.value.integerVal;
}

bp::Object * 
bp::Integer::clone() const
{
    return new Integer(*this);
}

bp::Integer::operator long long() const 
{
    return value();
}

bp::CallBack::CallBack(BPCallBack cb) : Integer(cb)
{
    e.type = BPTCallBack;
}

bp::Object * 
bp::CallBack::clone() const
{
    return new CallBack(*this);
}

bp::CallBack::~CallBack()
{
}

bp::Double::Double(BPDouble num)
    : Object(BPTDouble)
{
    e.value.doubleVal = num;
}

bp::Double::~Double()
{
}

BPDouble
bp::Double::value() const
{
    return e.value.doubleVal;
}

bp::Object *
bp::Double::clone() const
{
    return new Double(*this);
}


bp::Double::operator double() const 
{
    return value();
}

bp::List::List() : Object(BPTList)
{
    e.value.listVal.size = 0;
    e.value.listVal.elements = NULL;
}

bp::List::List(const List & other) : Object(BPTList)
{
    e.value.listVal.size = 0;
    e.value.listVal.elements = NULL;

    for (unsigned int i = 0; i < other.size(); i++) {
        append(other.value(i)->clone());
    }
}

bp::List &
bp::List::operator= (const List & other)
{
    // release
    for (unsigned int i = 0; i < values.size(); i++) delete values[i];
    if (e.value.listVal.elements != NULL) free(e.value.listVal.elements);

    // reinitialize
    memset((void *) &e, 0, sizeof(e));
    values.clear();
    e.type = BPTList;    

    // populate
    for (unsigned int i = 0; i < other.size(); i++) {
        append(other.value(i)->clone());
    }
    
    return *this;
}

bp::List::~List()
{
    for (unsigned int i = 0; i < values.size(); i++)
    {
        delete values[i];
    }
    
    if (e.value.listVal.elements != NULL) 
    {
        free(e.value.listVal.elements);
    }
}

unsigned int
bp::List::size() const
{
    return e.value.listVal.size;
}

const bp::Object *
bp::List::value(unsigned int i) const
{
    BPASSERT(e.value.listVal.size == values.size());
    if (i >= e.value.listVal.size) return NULL;
    return values[i];
}

const bp::Object &
bp::List::operator[](unsigned int index) const
{
    const bp::Object * v = value(index);
    if (v == NULL) {
        BP_THROW("no such element in list, range error");
    }
    
    return *v;
}

void
bp::List::append(bp::Object * object)
{
    BPASSERT(object != NULL);
    values.push_back(object);
    e.value.listVal.size++;
    e.value.listVal.elements = 
        (BPElement **) realloc(e.value.listVal.elements,
                               sizeof(BPElement *) * e.value.listVal.size);
    e.value.listVal.elements[e.value.listVal.size - 1] =
        (BPElement *) object->elemPtr();
}

bp::Object *
bp::List::clone() const
{
    return new bp::List(*this);
}

bp::List::operator std::vector<const bp::Object *>() const
{
    std::vector<const Object *> v;
    for (unsigned int i = 0; i < size(); i++) v.push_back(value(i));
    return v;
}
