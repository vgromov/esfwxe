#include <esfwxe/target.h>
#include <esfwxe/irq.h>

#if LPC23XX == 1
    #include <esfwxe/lpc23xx/target_hw.cc>
#endif

#if LPC214X == 1
    #include <esfwxe/lpc214x/target_hw.cc>
#endif

#if LPC2103 == 1
    #include <esfwxe/lpc2103/target_hw.cc>
#endif

#if LPC11XX == 1
    #include <esfwxe/lpc11xx/target_hw.cc>
#endif

#if LPC13XX == 1
    #include <esfwxe/lpc13xx/target_hw.cc>
#endif
