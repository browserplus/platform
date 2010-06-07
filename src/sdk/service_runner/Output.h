/* A wrapper that should be used for all output.  This allows for proper
 * JSON formatted output */
   

#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include "BPUtils/bptypeutil.h"

#include <string>

namespace output 
{
    typedef enum {
        T_INFO,    // optional informational output
        T_ERROR,   // fatal error
        T_RESULTS, // results from a command 
        T_CALLBACK // callback invoked during command execution
    } type;
    
    // set "slave mode" turning all output into json formatted 
    // messages
    void setSlaveMode();

    // output a string message 
    void puts(output::type msgType, const std::string & msg);
    void puts(output::type msgType, const bp::Object * obj);
};


#endif
