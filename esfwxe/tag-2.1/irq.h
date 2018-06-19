#ifndef _common_irq_h_
#define _common_irq_h_

void vicReset(void);
void vicInit(void);
// priority parameter is not used in some MCU cores
esBL irqInstall( esU32 IntNumber, void *HandlerAddr, esU32 priority );
esBL irqUninstall( esU32 IntNumber );

// MCU HW specific interrupt defines
#if LPC2103 == 1
	#include <esfwxe/lpc2103/irq_hw.h>
#endif

#if LPC23XX == 1
	#include <esfwxe/lpc23xx/irq_hw.h>
#endif

#if LPC214X == 1
	#include <esfwxe/lpc214x/irq_hw.h>
#endif

#endif 
