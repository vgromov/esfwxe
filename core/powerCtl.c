#include <esfwxe/target.h>
#pragma hdrstop

#include "powerCtl.h"

// include particular hardware wdt driver implementation
#if LPC23XX == 1
    #include "lpc23xx/powerctl_hw.cc"
#endif

#if LPC214X == 1
    #include "lpc214x/powerctl_hw.cc"
#endif

#if LPC2103 == 1
    #include "lpc2103/powerctl_hw.cc"
#endif

#if LPC11XX == 1
    #include "lpc11xx/powerctl_hw.cc"
#endif

#if LPC13XX == 1
    #include "lpc13xx/powerctl_hw.cc"
#endif
