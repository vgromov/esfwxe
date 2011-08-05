#include <common/target.h>
#include <common/irq.h>

#if LPC23XX == 1
	#include <common/lpc23xx/target_hw.cc>
#endif

#if LPC214X == 1
	#include <common/lpc214x/target_hw.cc>
#endif

#if LPC2103 == 1
	#include <common/lpc2103/target_hw.cc>
#endif
