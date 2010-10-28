/* A wrapper that should be used for all output.  This allows for proper
 * JSON formatted output */
   

#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include <string>
#include "BPUtils/bptypeutil.h"
#include "platform_utils/ServiceDescription.h"


namespace output 
{
    typedef enum {
        T_INFO,    // optional informational output
        T_ERROR,   // fatal error
        T_RESULTS, // results from a command 
        T_CALLBACK,// callback invoked during command execution
        T_WARNING, // non-fatal error, still informational
        T_PROMPT   // service emitted a prompt, as in something that would be displayed
                   // to the end user
    } type;
    
    // set "slave mode" turning all output into json formatted 
    // messages
    void setSlaveMode();
    bool getSlaveMode();

    // output a string message 
    void puts(output::type msgType, const std::string & msg);
    void puts(output::type msgType, const bp::Object * obj);
    void puts(output::type msgType, const bp::service::Description & desc);
};


#endif
