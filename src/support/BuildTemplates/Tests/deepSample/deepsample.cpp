/**
 * a test of "deep" samples.  meaning samples that use subdirectories. 
 */

#include <iostream>
#include "level1/level1.h"
#include "level1/level2/level2.h"

int
main(int argc, char ** argv)
{
    std::cout << "I'm such a _deep_ sample" << std::endl;
    level1();
    level2();    
    return 0;
}
