#include <esfwxe/target.h>
#include <esfwxe/irq.h>

#if LPC2103 == 1
    #include <esfwxe/lpc2103/irq_hw.cc>
#endif

#if LPC23XX == 1
    #include <esfwxe/lpc23xx/irq_hw.cc>
#endif

#if LPC214X == 1
    #include <esfwxe/lpc214x/irq_hw.cc>
#endif
