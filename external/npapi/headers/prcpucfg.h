/* Different platforms have different cpu configs
 */
#if defined(_WINDOWS)
#include "prcpucfg_win.h"
#elif defined(_APPLE)
#include "prcpucfg_mac.h"
#else
#error Unknown platform
#endif

