#ifndef _wdt_h_
#define _wdt_h_

#ifdef __cplusplus
	extern "C" {
#endif

// watchdog driver abstraction API
//
void wdtInit(void);
void wdtReset(void);

#ifdef __cplusplus
	}
#endif

#endif // _wdt_h_
