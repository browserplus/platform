/* A wrapper that should be used for all output.  This allows for proper
 * JSON formatted output */

#include "Output.h"

#include <sstream>

static bool s_slaveMode = false;

void output::setSlaveMode()
{
    s_slaveMode = true;
}

void
output::puts(output::type msgType, const bp::Object * obj)
{
    if (s_slaveMode) {
        std::string t;
        switch (msgType) {
            case T_INFO:     t = "info"; break;
            case T_ERROR:    t = "error"; break;
            case T_RESULTS:  t = "results"; break;              
            case T_CALLBACK: t = "callback"; break;
        }
        bp::Map m;
        m.add("type", new bp::String(t));
        m.add("msg", obj->clone());
        std::cout << m.toPlainJsonString(false) << std::endl;
        std::cout.flush();
    } else {
        std::string m;
        if (obj->type() == BPTString) {
            std::stringstream ss;
            ss << ((bp::String *) obj)->value() << std::endl;
            m = ss.str();
        } else {
            m = obj->toPlainJsonString(true);
        }
        if (msgType == T_ERROR) {
            std::cerr << m;            
        } else {
            std::cout << m;
        }
    }
}

void
output::puts(output::type msgType, const std::string & msg)
{
    bp::String s = bp::String(msg);
    puts(msgType, &s);
}