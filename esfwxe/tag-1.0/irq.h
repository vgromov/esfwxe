#ifndef _common_irq_h_
#define _common_irq_h_

void init_VIC( void );
// priority parameter is not used in some MCU cores
BOOL install_irq( DWORD IntNumber, void *HandlerAddr, DWORD priority );
BOOL uninstall_irq( DWORD IntNumber );

// MCU HW specific interrupt defines
#if LPC2103 == 1
	#include <common/lpc2103/irq_hw.h>
#endif

#if LPC23XX == 1
	#include <common/lpc23xx/irq_hw.h>
#endif

#if LPC214X == 1
	#include <common/lpc214x/irq_hw.h>
#endif

#endif 
