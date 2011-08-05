#include <common/target.h>
#include <common/irq.h>

#if LPC2103 == 1
	#include <common/lpc2103/irq_hw.cc>
#endif

#if LPC23XX == 1
	#include <common/lpc23xx/irq_hw.cc>
#endif

#if LPC214X == 1
	#include <common/lpc214x/irq_hw.cc>
#endif
