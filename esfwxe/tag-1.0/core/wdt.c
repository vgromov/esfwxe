#include <common/target.h>
#pragma hdrstop

#include "wdt.h"

// include particular hardware wdt driver implementation
#if LPC23XX == 1
	#include "lpc23xx/wdt_hw.cc"
#endif

#if LPC214X == 1
	#include "lpc214x/wdt_hw.cc"
#endif

#if LPC2103 == 1
	#include "lpc2103/wdt_hw.cc"
#endif
